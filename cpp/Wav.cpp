// Wav.cpp

#pragma execution_character_set("utf-8") 

//---------------------------------------------------------------------------//
//
// 音声データクラス
//   Copyright (C) 2013-2014 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "riff.h"
#include "DebugPrint.hpp"
#include "CriticalSection.hpp"

#include "Wav.hpp"

//---------------------------------------------------------------------------//
//
// Utility Function
//
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
// Internal Functions
//
//---------------------------------------------------------------------------//

static void __stdcall WriteFile64(HANDLE hFile, uint8_t* data, uint64_t data_size)
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

static void __stdcall memcpy64(void* dst, void* src, uint64_t size)
{
    for ( uint64_t pointer = 0; pointer < size; pointer += SIZE_MAX )
    {
        // (SIZE_MAX) byte ずつ 書き出し
        const auto size32 = (size_t)min(SIZE_MAX, size - pointer);
        ::memcpy(dst, (uint8_t*)src + pointer, size32);
    }
}

//---------------------------------------------------------------------------//

static bool __stdcall ReadHeader(uint8_t* p)
{
    char     chunkId[4];
    uint32_t chunkSize;
    char     riffType[4];

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
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------//

static void _stdcall ReadFormatChunk
(
    uint8_t* p, WAVEFORMATEXTENSIBLE* format
)
{
    WORD tag = WAVE_FORMAT_UNKNOWN;
    memcpy(&tag, p, sizeof(tag));
    console_out(TEXT("wFormatTag: %d"), tag);

    if ( tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_IEEE_FLOAT )
    {
        memcpy(format, p, sizeof(PCMWAVEFORMAT));
    }
    else if ( tag == WAVE_FORMAT_EXTENSIBLE )
    {
        memcpy(format, p, sizeof(WAVEFORMATEXTENSIBLE));
    }
    else
    {
        console_out(TEXT("UNKNOWN FORMAT"));
    }
    console_out(TEXT("%6d Hz"),   format->Format.nSamplesPerSec);
    console_out(TEXT("%6d bits"), format->Format.wBitsPerSample);
    console_out(TEXT("%6d bits"), format->Samples.wValidBitsPerSample);
    console_out(TEXT("%6d ch"),   format->Format.nChannels);
}

//---------------------------------------------------------------------------//

static void __stdcall WriteHeader
(
    HANDLE hFile, uint64_t dataSize
)
{
    DWORD    cb_written;
    uint32_t chunkSize;

    if ( dataSize < UINT32_MAX )
    {
        chunkSize = sizeof(RiffChunk) + sizeof(WAVEFORMATEXTENSIBLE) + (uint32_t)dataSize;
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
}

//---------------------------------------------------------------------------//

static void __stdcall WriteFormatChunk
(
    HANDLE hFile, const WAVEFORMATEXTENSIBLE* format
)
{
    static const uint32_t chunkSize = sizeof(WAVEFORMATEXTENSIBLE);

    DWORD cb_written;

    ::WriteFile(hFile, chunkId_fmt, sizeof(chunkId_fmt), &cb_written, nullptr);
    ::WriteFile(hFile, &chunkSize,  sizeof(chunkSize),   &cb_written, nullptr);
    ::WriteFile(hFile, format,      chunkSize,           &cb_written, nullptr);
}

//---------------------------------------------------------------------------//

static void __stdcall WriteChunk
(
    HANDLE hFile, const char chunkId[4], uint32_t chunkSize, uint8_t* chunkData
)
{
    DWORD cb_written;

    ::WriteFile(hFile, chunkId,    4 * sizeof(char),  &cb_written, nullptr);
    ::WriteFile(hFile, &chunkSize, sizeof(chunkSize), &cb_written, nullptr);
    ::WriteFile(hFile, chunkData,  chunkSize,         &cb_written, nullptr);
}

//---------------------------------------------------------------------------//
//
// Pimpl イディオム
//
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

    bool     __stdcall ReadAllChunks();
    void     __stdcall ReadDataSize64Chunk(uint8_t* p, uint32_t chunkSize);

    bool     __stdcall WriteAllChunks(HANDLE hFile);
    void     __stdcall WriteDataSize64Chunk(HANDLE hFile);
    void     __stdcall WriteChunk64(HANDLE hFile, const char chunkId[4], uint8_t* chunkData);

    uint8_t* __stdcall ForwardPointer(uint8_t* p, const char chunkId[4], uint32_t chunkSize);
    uint64_t __stdcall LookUpSizeTable(const char chunkId[4]);
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

bool __stdcall Wav::Impl::ReadAllChunks()
{
    if ( riff_size < sizeof(RiffChunk) )
    {
        console_out(TEXT("RIFF size is too small"));
        return false;
    }

    auto p = pView;

    // RIFFチャンクの読み込み
    const auto ret = ReadHeader(p);
    if ( false == ret )
    {
        return ret;
    }
    p = p + sizeof(RiffChunk);

    char     chunkId[4];
    uint32_t chunkSize;

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
            ReadDataSize64Chunk(p + 8, chunkSize);
        }
        else if ( 0 == ::memcmp(chunkId, chunkId_fmt, sizeof(chunkId)) )
        {
            // 'fmt ' chunk
            ReadFormatChunk(p + 8, &format);
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
        p = ForwardPointer(p, chunkId, chunkSize);
        if ( nullptr == p )
        {
            return false;
        }

        console_out(TEXT("Reading chunk end"));
    }

    return true;
}

//---------------------------------------------------------------------------//

void _stdcall Wav::Impl::ReadDataSize64Chunk
(
    uint8_t* p, uint32_t chunkSize
)
{
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

//---------------------------------------------------------------------------//

void __stdcall Wav::Impl::WriteDataSize64Chunk
(
    HANDLE hFile
)
{
    DWORD cb_written;

    const uint32_t table_size = table_length * sizeof(ChunkSize64);
    const uint32_t chunkSize  = sizeof(DataSize64Chunk) + table_size;
    if ( riff_size == 0 )
    {
        riff_size = sizeof(RF64Chunk) + sizeof(WAVEFORMATEXTENSIBLE) + data_size;
    }

    ::WriteFile(hFile, chunkId_ds64,  sizeof(chunkId_ds64), &cb_written, nullptr);
    ::WriteFile(hFile, &chunkSize,    sizeof(chunkSize),    &cb_written, nullptr);
    ::WriteFile(hFile, &riff_size,    sizeof(riff_size),    &cb_written, nullptr);
    ::WriteFile(hFile, &data_size,    sizeof(data_size),    &cb_written, nullptr);
    ::WriteFile(hFile, &sample_count, sizeof(sample_count), &cb_written, nullptr);
    ::WriteFile(hFile, &table_length, sizeof(table_length), &cb_written, nullptr);
    if ( table_ds64 )
    {
        ::WriteFile(hFile, table_ds64, table_size, &cb_written, nullptr);
    }
}

//---------------------------------------------------------------------------//

void __stdcall Wav::Impl::WriteChunk64
(
    HANDLE hFile, const char chunkId[4], uint8_t* chunkData
)
{
    static const uint32_t chunkSize = UINT32_MAX;

    DWORD cb_written;

    ::WriteFile(hFile,    chunkId,  4 * sizeof(char), &cb_written, nullptr);
    ::WriteFile(hFile, &chunkSize, sizeof(chunkSize), &cb_written, nullptr);

    if ( 0 == strncmp(chunkId, chunkId_data, sizeof(chunkId)) )
    {
        WriteFile64(hFile, chunkData, data_size);
    }
    else if ( table_ds64 )
    {
        WriteFile64(hFile, chunkData, LookUpSizeTable(chunkId));
    }
    else
    {
        console_out(TEXT("Size table look up error"));
    }
}

//---------------------------------------------------------------------------//

bool __stdcall Wav::Impl::WriteAllChunks(HANDLE hFile)
{
    char     chunkId[4];
    uint32_t chunkSize;

    auto p = pView + sizeof(RiffChunk);
    while ( p < pView + riff_size )
    {
        console_out(TEXT("Writing chunk begin"));

        ::memcpy(chunkId,    p,     sizeof(chunkId));
        ::memcpy(&chunkSize, p + 4, sizeof(chunkSize));
        console_out(TEXT("Chunk Id:   '%c%c%c%c'"), chunkId[0], chunkId[1], chunkId[2], chunkId[3]);
        console_out(TEXT("Chunk size: %d bytes"), chunkSize);

        if ( 0 == ::memcmp(chunkId, chunkId_ds64, sizeof(chunkId)) )
        {
            WriteDataSize64Chunk(hFile);
        }
        else if ( chunkSize < UINT32_MAX )
        {
            WriteChunk(hFile, chunkId, chunkSize, p + 8);
        }
        else
        {
            WriteChunk64(hFile, chunkId, p + 8);
        }

        p = ForwardPointer(p, chunkId, chunkSize);
        if ( nullptr == p )
        {
            return false;
        }

        console_out(TEXT("Writing chunk end"));
    }

    return true;
}

//---------------------------------------------------------------------------//

uint8_t* __stdcall Wav::Impl::ForwardPointer
(
    uint8_t* p, const char chunkId[4], uint32_t chunkSize
)
{
    if ( 8 < chunkSize && chunkSize < UINT32_MAX )
    {
        return p + 4 * sizeof(char) + sizeof(chunkSize) + chunkSize;
    }
    else
    {
        if ( 0 == strncmp(chunkId, chunkId_data, sizeof(chunkId)) )
        {
            return p + 4 * sizeof(char) + sizeof(chunkSize) + data_size;
        }
        else if ( table_ds64 )
        {
            return p + 4 * sizeof(char) + sizeof(chunkSize) + LookUpSizeTable(chunkId);
        }
        else
        {
            console_out(TEXT("Size table look up error"));
            return nullptr;
        }
    }
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
        return E_INVALIDARG;
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

    const auto ret = pimpl->ReadAllChunks();

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

    HRESULT hr = E_FAIL;

    const auto hFile = ::CreateFileW
    (
        path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        console_out(TEXT("CreateFile() failed"), TEXT(__FILE__));
        goto CLOSE;
    }

    bool ret = true;

    pimpl->cs.lock();
    {
        WriteHeader(hFile, pimpl->data_size);
        if ( pimpl->data_size < UINT32_MAX )
        {
            DataSize64Chunk junk { };
            WriteChunk(hFile, chunkId_JUNK, sizeof(junk), (uint8_t*)&junk);
        }

        if ( pimpl->hFile != INVALID_HANDLE_VALUE )
        {
            ret = pimpl->WriteAllChunks(hFile);
        }
        else
        {
            if ( pimpl->data_size < UINT32_MAX )
            {
                WriteFormatChunk(hFile, &pimpl->format);
                WriteChunk(hFile, chunkId_data, (uint32_t)(pimpl->data_size), pimpl->data);
            }
            else
            {
                pimpl->WriteDataSize64Chunk(hFile);
                WriteFormatChunk(hFile, &pimpl->format);
                pimpl->WriteChunk64(hFile, chunkId_data, pimpl->data);
            }
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