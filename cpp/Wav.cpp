// Wav.cpp

#pragma execution_character_set("utf-8") 

//---------------------------------------------------------------------------//
//
// 音声データクラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "riff.h"
#include "DebugPrint.hpp"

#include "Wav.hpp"

//---------------------------------------------------------------------------//
//
// Utility Function
//
//---------------------------------------------------------------------------//

void __stdcall WriteFile64(HANDLE hFile, uint8_t* data, uint64_t data_size)
{
    DWORD cb_written;

    for ( uint64_t pointer = 0; pointer < data_size; pointer += UINT32_MAX )
    {
        // 4,294,967,295 byte ずつ 書き出し
        const auto size = (DWORD)min(UINT32_MAX, data_size - pointer);
        ::WriteFile(hFile, data + pointer, size, &cb_written, nullptr);
    }
}

//---------------------------------------------------------------------------//

void __stdcall memcpy64(void* dst, void* src, uint64_t size)
{
    for ( uint64_t pointer = 0; pointer < size; pointer += SIZE_MAX )
    {
        // (SIZE_MAX) byte ずつ 書き出し
        const auto size32 = (size_t)min(SIZE_MAX, size - pointer);
        ::memcpy(dst, (uint8_t*)src + pointer, size32);
    }
}

//---------------------------------------------------------------------------//

uint32_t __stdcall MaskChannelMask(uint16_t channels)
{
    switch ( channels )
    {
        case 1: // monaural
        {
            return SPEAKER_FRONT_CENTER;
        }
        case 2: // stereo
        {
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
        }
        case 4:
        {
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
                   SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT;
        }
        case 6: // 5.1 ch
        {
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
                   SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
                   SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT;
        }
        case 8: // 7.1 ch
        {
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
                   SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
                   SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT |
                   SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
        }
        default:
        {
            return 0;
        }
    }
}

//---------------------------------------------------------------------------//
//
// Structures for Inner Data
//
//---------------------------------------------------------------------------//

class CriticalSection
{
public:
    CriticalSection()  { ::InitializeCriticalSection(&cs); }
    ~CriticalSection() { ::DeleteCriticalSection(&cs); }
    void __stdcall lock()   { ::EnterCriticalSection(&cs); }
    void __stdcall unlock() { ::LeaveCriticalSection(&cs); }

private:
    CRITICAL_SECTION cs;
};

//---------------------------------------------------------------------------//

struct Wav::Impl
{
    CriticalSection cs;

    WAVEFORMATEXTENSIBLE format;
    WCHAR    path[MAX_PATH];
    HANDLE   hFile;
    HANDLE   hMap;
    uint8_t* pView;

    uint64_t     riff_size;
    uint64_t     sample_count;
    uint32_t     table_length;
    ChunkSize64* table_ds64;
    uint64_t     data_size;
    uint8_t*     data;

    Impl();
    ~Impl();

    void     __stdcall Init();
    bool     __stdcall ReadChunks ();
    bool     __stdcall WriteChunks(HANDLE hFile);
    uint64_t __stdcall LookUpSizeTable(const char* chunkId);
};

//---------------------------------------------------------------------------//

Wav::Impl::Impl()
{
    this->Init();
}

//---------------------------------------------------------------------------//

Wav::Impl::~Impl()
{
}

//---------------------------------------------------------------------------//

void __stdcall Wav::Impl::Init()
{
    format.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE;
    format.Format.nChannels            = 0;
    format.Format.nSamplesPerSec       = 0;
    format.Format.nAvgBytesPerSec      = 0;
    format.Format.nBlockAlign          = 0;
    format.Format.wBitsPerSample       = 0;
    format.Format.cbSize               = 22;
    format.Samples.wValidBitsPerSample = 0;
    format.dwChannelMask               = 0;
    format.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

    path[0] = L'\0';

    hFile = INVALID_HANDLE_VALUE;
    hMap  = nullptr;
    pView = nullptr;

    riff_size    = 0;
    sample_count = 0;
    table_length = 0;
    table_ds64   = nullptr;
    data_size    = 0;
    data         = nullptr;
}

//---------------------------------------------------------------------------//

bool __stdcall Wav::Impl::ReadChunks()
{
    if ( riff_size < sizeof(RiffChunk) )
    {
        console_out(TEXT("RIFF size is too small"));
        return false;
    }

    auto p = pView;

    char     chunkId[4];
    uint32_t chunkSize;
    char     riffType[4];

    // RIFFチャンクの読み込み
    memcpy(chunkId,    p,     sizeof(chunkId));
    memcpy(&chunkSize, p + 4, sizeof(chunkSize));
    memcpy(riffType,   p + 8, sizeof(riffType));
    console_out(TEXT("Chunk Id:   '%c%c%c%c'"), chunkId[0], chunkId[1], chunkId[2], chunkId[3]);
    console_out(TEXT("Chunk size: %d bytes"), chunkSize);
    console_out(TEXT("RIFF type:  '%c%c%c%c'"), riffType[0], riffType[1], riffType[2], riffType[3]);
    if ( 0 == ::memcmp(chunkId, chunkId_RF64, sizeof(chunkId)) )
    {
        if ( chunkSize != UINT32_MAX )
        {
            console_out(TEXT("Invalid chunk size as RF64 format"));
            return false;
        }
    }
    else if ( ::memcmp(chunkId, chunkId_RIFF, sizeof(chunkId)) )
    {
        console_out(TEXT("Not RIFF nor RF64"));
        return false;
    }
    if ( ::memcmp(riffType, riffType_WAVE, sizeof(riffType)) )
    {
        console_out(TEXT("Not WAVE"));
    }
    p = p + sizeof(chunkId) + sizeof(chunkSize) + sizeof(riffType);

    // RIFFサブチャンクの読み込み
    while ( p < pView + riff_size )
    {
        console_out(TEXT("Reading chunk begin"));

        // チャンクIDの読み込み
        ::memcpy(chunkId,    p,     sizeof(chunkId));
        ::memcpy(&chunkSize, p + 4, sizeof(chunkSize));
        console_out(TEXT("Chunk Id:   '%c%c%c%c'"), chunkId[0], chunkId[1], chunkId[2], chunkId[3]);
        console_out(TEXT("Chunk size: %d bytes"), chunkSize);

        // チャンクデータの読み込み
        if ( 0 == ::memcmp(chunkId, chunkId_ds64, sizeof(chunkId)) )
        {
            // 'ds64' chunk
            if ( chunkSize > sizeof(DataSize64Chunk) )
            {
                // チャンクサイズ情報が格納されているとき
                DataSize64Chunk chunk;
                ::memcpy(&chunk, p, sizeof(chunk));
                riff_size    = chunk.riffSize;
                data_size    = chunk.dataSize;
                sample_count = chunk.sampleCount;
                table_length = chunk.tableLength;

                table_ds64 = new ChunkSize64[table_length];
                ::memcpy(table_ds64, p + sizeof(DataSize64Chunk), table_length * sizeof(ChunkSize64));
            }
            else
            {
                // 最小限の情報しか格納されていないとき
                DataSize64ChunkLight chunk;
                ::memcpy(&chunk, p, sizeof(chunk));
                riff_size = chunk.riffSize;
                data_size = chunk.dataSize;
            }
            console_out(TEXT("RIFF size: %d"), riff_size);
            console_out(TEXT("Data size: %d"), data_size);
        }
        else if ( 0 == ::memcmp(chunkId, chunkId_fmt, sizeof(chunkId)) )
        {
            // 'fmt ' chunk
            WORD tag = WAVE_FORMAT_UNKNOWN;
            memcpy(&tag, p + 8, sizeof(tag));
            console_out(TEXT("wFormatTag: %d"), tag);

            // フォーマット情報の読み込み
            if ( tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_IEEE_FLOAT )
            {
                memcpy(&format, p + 8, sizeof(PCMWAVEFORMAT));
            }
            else if ( tag == WAVE_FORMAT_EXTENSIBLE )
            {
                memcpy(&format, p + 8, sizeof(WAVEFORMATEXTENSIBLE));
            }
            else
            {
                console_out(TEXT("UNKNOWN FORMAT"));
            }
            console_out(TEXT("%6d Hz"),   format.Format.nSamplesPerSec);
            console_out(TEXT("%6d bits"), format.Format.wBitsPerSample);
            console_out(TEXT("%6d bits"), format.Samples.wValidBitsPerSample);
            console_out(TEXT("%6d ch"),   format.Format.nChannels);
        }
        else if ( 0 == strncmp(chunkId, chunkId_data, sizeof(chunkId)) )
        {
            // 'data' chunk
            if ( chunkSize < UINT32_MAX )
            {
                data_size = chunkSize;
                console_out(TEXT("Data size:  %d bytes"), data_size);
            }
            data = p + 8;
        }
        else
        {
            console_out(TEXT("Unknown chunk type"));
        }

        // ポインタを次のチャンクまで進める
        if ( 8 < chunkSize && chunkSize < UINT32_MAX )
        {
            p = p + sizeof(chunkId) + sizeof(chunkSize) + chunkSize;
        }
        else
        {
            if ( 0 == strncmp(chunkId, chunkId_data, sizeof(chunkId)) )
            {
                p = p + sizeof(chunkId) + sizeof(chunkSize) + data_size;
            }
            else if ( table_ds64 )
            {
                const auto chunkSize64 = this->LookUpSizeTable(chunkId);
                if ( chunkSize64 == 0 )
                {
                    console_out(TEXT("Size table look up error"));
                    return false;
                }
                p = p + sizeof(chunkId) + sizeof(chunkSize) + chunkSize64;
            }
            else
            {
                console_out(TEXT("Size table look up error"));
                return false;
            }
        }

        console_out(TEXT("Reading chunk end"));
    }

    return true;
}

//---------------------------------------------------------------------------//

bool __stdcall Wav::Impl::WriteChunks(HANDLE hFile)
{
    auto p = pView + sizeof(RiffChunk);

    DWORD    cb_written;
    char     chunkId[4];
    uint32_t chunkSize;

    if ( data_size < UINT32_MAX )
    {
        chunkSize = (uint32_t)data_size;
        ::WriteFile(hFile, chunkId_RIFF,  sizeof(chunkId_RIFF),  &cb_written, nullptr);
        console_out(TEXT("Chunk Id:   '%c%c%c%c'"), chunkId_RIFF[0], chunkId_RIFF[1], chunkId_RIFF[2], chunkId_RIFF[3]);
    }
    else
    {
        chunkSize = UINT32_MAX;
        ::WriteFile(hFile, chunkId_RF64,  sizeof(chunkId_RF64),  &cb_written, nullptr);
        console_out(TEXT("Chunk Id:   '%c%c%c%c'"), chunkId_RF64[0], chunkId_RF64[1], chunkId_RF64[2], chunkId_RF64[3]);
    }
    ::WriteFile(hFile, &chunkSize,    sizeof(chunkSize),     &cb_written, nullptr);
    ::WriteFile(hFile, riffType_WAVE, sizeof(riffType_WAVE), &cb_written, nullptr);
    console_out(TEXT("Chunk size: %d bytes"), chunkSize);
    console_out(TEXT("RIFF type:  '%c%c%c%c'"), riffType_WAVE[0], riffType_WAVE[1], riffType_WAVE[2], riffType_WAVE[3]);

    while ( p < pView + riff_size )
    {
        console_out(TEXT("Writing chunk begin"));

        ::memcpy(chunkId,    p,     sizeof(chunkId));
        ::memcpy(&chunkSize, p + 4, sizeof(chunkSize));
        console_out(TEXT("Chunk Id:   '%c%c%c%c'"), chunkId[0], chunkId[1], chunkId[2], chunkId[3]);
        console_out(TEXT("Chunk size: %d bytes"), chunkSize);

        ::WriteFile(hFile, chunkId,    sizeof(chunkId),   &cb_written, nullptr);
        ::WriteFile(hFile, &chunkSize, sizeof(chunkSize), &cb_written, nullptr);

        if ( 0 == ::memcmp(chunkId, chunkId_ds64, sizeof(chunkId)) )
        {
            uint32_t table_size = table_length * sizeof(ChunkSize64);

            chunkSize = sizeof(DataSize64Chunk) + table_size;
            {
                ::WriteFile(hFile, &riff_size,    sizeof(riff_size),    &cb_written, nullptr);
                ::WriteFile(hFile, &data_size,    sizeof(data_size),    &cb_written, nullptr);
                ::WriteFile(hFile, &sample_count, sizeof(sample_count), &cb_written, nullptr);
                ::WriteFile(hFile, &table_length, sizeof(table_length), &cb_written, nullptr);
                if ( table_ds64 )
                {
                    ::WriteFile(hFile, table_ds64, table_size, &cb_written, nullptr);
                }
            }
        }
        else if ( chunkSize < UINT32_MAX )
        {
            ::WriteFile(hFile, p + 8, chunkSize, &cb_written, nullptr);
        }
        else
        {
            if ( 0 == strncmp(chunkId, chunkId_data, sizeof(chunkId)) )
            {
                WriteFile64(hFile, p + 8, data_size);
            }
            else if ( table_ds64 )
            {
                const auto chunkSize64 = this->LookUpSizeTable(chunkId);
                if ( chunkSize64 == 0 )
                {
                    console_out(TEXT("Size table look up error"));
                    return false;
                }
                WriteFile64(hFile, p + 8, chunkSize64);
            }
            else
            {
                console_out(TEXT("Size table look up error"));
                return false;
            }
        }

        // ポインタを次のチャンクまで進める
        if ( 8 < chunkSize && chunkSize < UINT32_MAX )
        {
            p = p + sizeof(chunkId) + sizeof(chunkSize) + chunkSize;
        }
        else
        {
            if ( 0 == strncmp(chunkId, chunkId_data, sizeof(chunkId)) )
            {
                p = p + sizeof(chunkId) + sizeof(chunkSize) + data_size;
            }
            else if ( table_ds64 )
            {
                const auto chunkSize64 = this->LookUpSizeTable(chunkId);
                if ( chunkSize64 == 0 )
                {
                    console_out(TEXT("Size table look up error"));
                    return false;
                }
                p = p + sizeof(chunkId) + sizeof(chunkSize) + chunkSize64;
            }
            else
            {
                console_out(TEXT("Size table look up error"));
                return false;
            }
        }

        console_out(TEXT("Writing chunk end"));
    }

    return true;
}

//---------------------------------------------------------------------------//

uint64_t __stdcall Wav::Impl::LookUpSizeTable(const char* chunkId)
{
    for ( size_t index = 0; index < table_length; ++index )
    {
        const auto chunk = table_ds64[index];
        if ( 0 == strncmp(chunkId, chunk.chunkId, sizeof(chunkId)) )
        {
            return chunk.chunkSize;
        }
    }
    return 0;
}

//---------------------------------------------------------------------------//
//
// Ctor / Dtor
//
//---------------------------------------------------------------------------//

Wav::Wav()
{
    console_out(TEXT("%s::ctor begin"), TEXT(__FILE__));

    pimpl = new Impl;

    this->AddRef();

    console_out(TEXT("%s::ctor end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

Wav::~Wav()
{
    console_out(TEXT("%s::dtor begin"), TEXT(__FILE__));

    this->Dispose();

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("%s::dtor end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//
//
// IUnknown Methods
//
//---------------------------------------------------------------------------//

HRESULT __stdcall Wav::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    console_out(TEXT("%s::QueryInterface() begin"), TEXT(__FILE__));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) )
    {
        console_out(TEXT("IID_IUnknown"));
        *ppvObject = static_cast<IUnknown*>(this);
    }
    else if ( IsEqualIID(riid, IID_IWav) )
    {
        console_out(TEXT("IID_IWav"));
        *ppvObject = static_cast<IWav*>(this);
    }
    else
    {
        console_out(TEXT("No such an interface"));
        console_out(TEXT("%s::QueryInterface() end"), TEXT(__FILE__));
        return E_NOINTERFACE;
    }

    this->AddRef();

    console_out(TEXT("%s::QueryInterface() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall Wav::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::AddRef(): %d -> %d"), TEXT(__FILE__), cRef - 1, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Wav::Release()
{
    if ( m_cRef < 1 )
    {
        console_out(TEXT("%s::Release() %d"), TEXT(__FILE__), m_cRef);
        return m_cRef;
    }

    LONG cRef = ::InterlockedDecrement(&m_cRef);

    console_out(TEXT("%s::Release(): %d -> %d"), TEXT(__FILE__), cRef + 1, cRef);

    if ( cRef == 0 )
    {
        console_out(TEXT("%s::delete begin"), TEXT(__FILE__));
        {
            delete this;
        }
        console_out(TEXT("%s::delete end"), TEXT(__FILE__));
    }

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//
//
// Properties
//
//---------------------------------------------------------------------------//

WAVEFORMATEXTENSIBLE* __stdcall Wav::format() const
{
    return &pimpl->format;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall Wav::path() const
{
    return pimpl->path;
}

//---------------------------------------------------------------------------//

uint64_t __stdcall Wav::size() const
{
    return pimpl->data_size;
}

//---------------------------------------------------------------------------//

uint8_t* __stdcall Wav::data() const
{
    return pimpl->data;
}

//---------------------------------------------------------------------------//
//
// Methods
//
//---------------------------------------------------------------------------//

HRESULT __stdcall Wav::Create
(
    const WAVEFORMATEXTENSIBLE& format, uint64_t data_size
)
{
    console_out(TEXT("%s::Create() begin"), TEXT(__FILE__));

    if ( data_size >= UINT32_MAX && format.Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE )
    {
        console_out(TEXT("Invalid format: %d / %u bytes"), format.Format.wFormatTag, data_size);
        console_out(TEXT("%s::Create() end"), TEXT(__FILE__));
        return E_INVALIDARG;
    }

    if ( pimpl->data )
    {
        console_out(TEXT("Data already exists"));
        this->Dispose();
    }

    pimpl->cs.lock();

    ::memcpy(&pimpl->format, &format, sizeof(WAVEFORMATEXTENSIBLE));
    if ( format.dwChannelMask == 0 )
    {
        pimpl->format.dwChannelMask = MaskChannelMask(format.Format.nChannels);
    }

    // TODO: メモリマップドファイルにする
    pimpl->data_size = data_size;
    pimpl->data = new uint8_t[data_size];
    console_out(TEXT("Data size: %u"), pimpl->data_size);

    pimpl->cs.unlock();
    
    console_out(TEXT("%s::Create() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wav::Load(LPCWSTR path)
{
    console_out(TEXT("%s::Load() begin"), TEXT(__FILE__));

    if ( pimpl->data )
    {
        console_out(TEXT("Data already exists"));
        this->Dispose();
    }

    ::StringCchCopyW(pimpl->path, MAX_PATH, path);
    console_outW(L"%s", pimpl->path);

    pimpl->hFile = ::CreateFileW
    (
        path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( INVALID_HANDLE_VALUE == pimpl->hFile )
    {
        console_out(TEXT("CreateFile() failed"), TEXT(__FILE__));
        goto CLOSE;
    }

    pimpl->hMap = ::CreateFileMappingW
    (
        pimpl->hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr
    );
    if ( nullptr == pimpl->hMap )
    {
        console_out(TEXT("CreateFileMapping() failed"), TEXT(__FILE__));
        goto CLOSE;
    }

    pimpl->pView = (uint8_t*)::MapViewOfFile
    (
        pimpl->hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0
    );
    if ( nullptr == pimpl->pView )
    {
        console_out(TEXT("MapViewOfFile() failed"));
        goto CLOSE;
    }

    ::GetFileSizeEx(pimpl->hFile, (LARGE_INTEGER*)&pimpl->riff_size);
    console_out(TEXT("RIFF size: %u"), pimpl->riff_size);

    pimpl->cs.lock();

    const auto ret = pimpl->ReadChunks();

    pimpl->cs.unlock();

    if ( false == ret )
    {
        console_out(TEXT("ReadChunks() failed"));
        goto CLOSE;
    }

    console_out(TEXT("%s::Load() end"), TEXT(__FILE__));

    return S_OK;

CLOSE: this->Dispose();

    console_out(TEXT("%s::Load() end"), TEXT(__FILE__));

    return E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wav::Save(LPCWSTR path)
{
    console_out(TEXT("%s::Save() begin"), TEXT(__FILE__));
    console_outW(L"%s", path);

    if ( ::PathFileExistsW(path) )
    {
        auto ret = MessageBoxW(nullptr, L"上書きしますか？", path, MB_YESNO | MB_ICONQUESTION);
        if ( ret == IDNO )
        {
            console_out(TEXT("Canceled"));
            console_out(TEXT("%s::Save() end"), TEXT(__FILE__));
            return S_FALSE;
        }
    }

    HRESULT hr;

    const auto hFile = ::CreateFileW
    (
        path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( INVALID_HANDLE_VALUE == pimpl->hFile )
    {
        console_out(TEXT("CreateFile() failed"), TEXT(__FILE__));
        hr = E_FAIL;
        goto CLOSE;
    }

    bool ret = true;

    pimpl->cs.lock();
    {
        if ( pimpl->hFile )
        {
            ret = pimpl->WriteChunks(hFile);
        }
        else
        {
            /// to be implemented
        }
    }
    pimpl->cs.unlock();

    if ( false == ret )
    {
        console_out(TEXT("WriteChunks() failed"));
        goto CLOSE;
    }

    hr = S_OK;

CLOSE: ::CloseHandle(hFile);

    console_out(TEXT("%s::Save() end"), TEXT(__FILE__));

    return hr;
}

//---------------------------------------------------------------------------//

size_t __stdcall Wav::Read(void* buffer, size_t offset, size_t buf_size)
{
    console_out(TEXT("%s::Read() begin"), TEXT(__FILE__));

    if ( offset >= pimpl->data_size )
    {
        console_out(TEXT("Invalid offset: %u"), offset);
        console_out(TEXT("%s::Read() end"), TEXT(__FILE__));
        return E_INVALIDARG;
    }

    pimpl->cs.lock();

    const auto size = min(buf_size, pimpl->data_size - offset);
    ::memcpy(buffer, pimpl->data + offset, size);

    pimpl->cs.unlock();

    console_out(TEXT("%s::Read() end"), TEXT(__FILE__));

    return size;
}

//---------------------------------------------------------------------------//

size_t __stdcall Wav::Write(void* buffer, size_t offset, size_t buf_size)
{
    console_out(TEXT("%s::Write() begin"), TEXT(__FILE__));

    if ( offset >= pimpl->data_size )
    {
        console_out(TEXT("Invalid offset: %u"), offset);
        console_out(TEXT("%s::Write() end"), TEXT(__FILE__));
        return E_INVALIDARG;
    }

    pimpl->cs.lock();

    const auto size = min(buf_size, pimpl->data_size - offset);
    ::memcpy(pimpl->data + offset, buffer, size);

    pimpl->cs.unlock();

    console_out(TEXT("%s::Write() end"), TEXT(__FILE__));

    return size;
}

//---------------------------------------------------------------------------//

IWav* __stdcall Wav::Clone()
{
    console_out(TEXT("%s::Clone() begin"), TEXT(__FILE__));

    auto wav = new Wav;

    if ( pimpl->hFile != INVALID_HANDLE_VALUE )
    {
        wav->Load(pimpl->path);
    }
    else
    {
        wav->pimpl->format = pimpl->format;
        wav->pimpl->data_size = pimpl->data_size;

        // TODO: メモリマップドファイルにする
        wav->pimpl->data = new uint8_t[pimpl->data_size];
        memcpy64(wav->pimpl->data, pimpl->data, pimpl->data_size);
    }

    console_out(TEXT("%s::Clone() end"), TEXT(__FILE__));

    return wav;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wav::Dispose()
{
    console_out(TEXT("%s::Dispose() begin"), TEXT(__FILE__));

    pimpl->cs.lock();

    if ( pimpl->table_ds64 )
    {
        delete pimpl->table_ds64;
    }
    if ( pimpl->pView )
    {
        ::UnmapViewOfFile(pimpl->pView);
    }
    if ( pimpl->hMap )
    {
        ::CloseHandle(pimpl->hMap);
    }

    // TODO: メモリマップドファイルにする
    if ( pimpl->hFile != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle(pimpl->hFile);
    }
    else if ( pimpl->data )
    {
        delete pimpl->data;
    }

    pimpl->Init();

    pimpl->cs.unlock();

    console_out(TEXT("%s::Dispose() end"), TEXT(__FILE__));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

// Wav.cpp