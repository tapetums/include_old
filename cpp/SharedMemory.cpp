﻿// SharedMemory.cpp

//---------------------------------------------------------------------------//
//
// 共有メモリをカプセル化するクラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>
#include <strsafe.h>

#include "MeteredSection.h"
#include "DebugPrint.hpp"
#include "GenerateUUIDString.hpp"

#include "SharedMemory.hpp"

//---------------------------------------------------------------------------//

static const char riffType_SHRD[4] = { 'S', 'H', 'R', 'D' };
static const char chunkId_sync [4] = { 's', 'y', 'n', 'c' };

//---------------------------------------------------------------------------//
//
// Struct for Inner Data
//
//---------------------------------------------------------------------------//

struct SharedMemory::Impl
{
    WCHAR    name[MAX_PATH];
    HANDLE   handle    = nullptr;
    INFO*    info      = nullptr;
    uint8_t* data      = nullptr;

    LPMETERED_SECTION msection  = nullptr;
    HANDLE            evt_block = nullptr;
    HANDLE            evt_read  = nullptr;
    HANDLE            evt_write = nullptr;
};

//---------------------------------------------------------------------------//
//
// Ctor / Dtor
//
//---------------------------------------------------------------------------//

SharedMemory::SharedMemory()
{
    console_out(TEXT("SharedMemory::ctor() begin"));

    pimpl = new Impl;

    pimpl->name[0] = L'\0';

    console_out(TEXT("SharedMemory::ctor() end"));
}

//---------------------------------------------------------------------------//

SharedMemory::~SharedMemory()
{
    console_out(TEXT("SharedMemory::dtor() begin"));

    if ( nullptr == pimpl )
    {
        console_out(TEXT("Already moved"));
        console_out(TEXT("SharedMemory::dtor() end"));
        return;
    }

    this->Close();

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("SharedMemory::dtor() end"));
}

//---------------------------------------------------------------------------//

SharedMemory::SharedMemory(const SharedMemory& lhs)
{
    console_out(TEXT("SharedMemory::ctor(copy) begin"));

    pimpl = new Impl;

    this->Open(lhs.pimpl->name);

    console_out(TEXT("SharedMemory::ctor(copy) end"));
}

//---------------------------------------------------------------------------//

SharedMemory::SharedMemory(SharedMemory&& rhs)
{
    console_out(TEXT("SharedMemory::ctor(move) begin"));

    pimpl = rhs.pimpl;

    rhs.pimpl = nullptr;

    console_out(TEXT("SharedMemory::ctor(move) end"));
}

//---------------------------------------------------------------------------//

SharedMemory& SharedMemory::operator =(const SharedMemory& lhs)
{
    console_out(TEXT("SharedMemory::operator= begin"));

    this->Open(lhs.pimpl->name);

    console_out(TEXT("SharedMemory::operator= end"));

    return *this;
}

//---------------------------------------------------------------------------//

SharedMemory& SharedMemory::operator =(SharedMemory&& rhs)
{
    console_out(TEXT("SharedMemory::operator= begin"));

    pimpl = rhs.pimpl;

    rhs.pimpl = nullptr;

    console_out(TEXT("SharedMemory::operator= end"));

    return *this;
}

//---------------------------------------------------------------------------//
//
// Properties
//
//---------------------------------------------------------------------------//

LPCWSTR __stdcall SharedMemory::name() const
{
    return pimpl->name;
}

//---------------------------------------------------------------------------//

bool __stdcall SharedMemory::is_open() const
{
    return (pimpl->handle != nullptr);
}

//---------------------------------------------------------------------------//

SharedMemory::INFO* __stdcall SharedMemory::info() const
{
    return pimpl->info;
}

//---------------------------------------------------------------------------//

uint8_t* __stdcall SharedMemory::data() const
{
    return pimpl->data;
}

//---------------------------------------------------------------------------//

uint64_t __stdcall SharedMemory::data_size() const
{
    return pimpl->info ? pimpl->info->chunk_ds64.dataSize : 0;
}

//---------------------------------------------------------------------------//

HANDLE __stdcall SharedMemory::evt_read() const
{
    return pimpl->evt_read;
}

//---------------------------------------------------------------------------//

HANDLE __stdcall SharedMemory::evt_write() const
{
    return pimpl->evt_write;
}

//---------------------------------------------------------------------------//
//
// Methods
//
//---------------------------------------------------------------------------//

// 新しい共有メモリを作成する
bool __stdcall SharedMemory::Create
(
    LPCWSTR  name,
    uint64_t data_size,
    LONG     max_reader_count
)
{
    console_out(TEXT("SharedMemory::Create() begin"));

    if ( this->is_open() )
    {
        this->Close();
    }

    // 作成する共有メモリの名前を決定
    if ( name )
    {
        // 名前をメンバ変数にコピー
        ::StringCchCopyW(pimpl->name, MAX_PATH, name);
    }
    else
    {
        // ランダムな名前を生成
        wchar_t buf[MAX_PATH];
        GenerateUUIDStringW(buf, MAX_PATH);
        ::StringCchCopyW(pimpl->name, MAX_PATH, buf);
    }
    console_outW(L"name: %s", pimpl->name);

    // 作成する共有メモリのサイズを求める
    LARGE_INTEGER size = { };
    size.QuadPart = sizeof(SharedMemory::INFO) + data_size;
    console_out(TEXT("data_size: %d + %d"), sizeof(SharedMemory::INFO), data_size);

    // 最大同時読み取り数を設定
    if ( max_reader_count < 1 )
    {
        // CPU の 論理コア数と同じに設定する
        SYSTEM_INFO si = { };
        ::GetNativeSystemInfo(&si);
        max_reader_count = si.dwNumberOfProcessors;
    }
    console_out(TEXT("max_reader_count: %d"), max_reader_count);

    // マッピングオブジェクトを作成
    pimpl->handle = ::CreateFileMappingW
    (
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT,
        size.HighPart, size.LowPart, pimpl->name
    );
    if ( nullptr == pimpl->handle )
    {
        console_out(TEXT("CreateFileMappingW() failed"));
        goto CLOSE;
    }

    // 共有メモリを自プロセスにマップ
    pimpl->info = (SharedMemory::INFO*)::MapViewOfFile
    (
        pimpl->handle, FILE_MAP_ALL_ACCESS, 0, 0, 0
    );
    if ( nullptr == pimpl->info )
    {
        console_out(TEXT("MapViewOfFile() failed"));
        goto CLOSE;
    }

    // データ部分の先頭アドレスを取得
    pimpl->data = (uint8_t*)pimpl->info + sizeof(SharedMemory::INFO);
    //console_out(TEXT("sizeof(SharedMemory::INFO): %u"), sizeof(SharedMemory::INFO));
    //console_out(TEXT("sizeof(RF64Chunk): %u"),          sizeof(RF64Chunk));
    //console_out(TEXT("sizeof(DataSize64Chunk): %u"),    sizeof(DataSize64Chunk));
    //console_out(TEXT("sizeof(SyncObjectChunk): %u"),    sizeof(SharedMemory::INFO::SyncObjectChunk));
    //console_out(TEXT("sizeof(DataChunk): %u"),          sizeof(DataChunk));

    // SharedMemory::INFO 構造体の中身を初期化
    {
        // 'RF64' chunk
        ::memcpy(pimpl->info->chunk_rf64.chunkId,  chunkId_RF64,  4 * sizeof(char));
        pimpl->info->chunk_rf64.chunkSize = UINT32_MAX;
        ::memcpy(pimpl->info->chunk_rf64.rf64Type, riffType_SHRD, 4 * sizeof(char));

        // 'ds64' chunk
        ::memcpy(pimpl->info->chunk_ds64.chunkId,  chunkId_ds64, 4 * sizeof(char));
        pimpl->info->chunk_ds64.chunkSize   = sizeof(DataSize64Chunk);
        pimpl->info->chunk_ds64.riffSize    = size.QuadPart;
        pimpl->info->chunk_ds64.dataSize    = data_size;
        pimpl->info->chunk_ds64.sampleCount = 0;
        pimpl->info->chunk_ds64.tableLength = 0;

        // 'sync' chunk
        ::memset(&pimpl->info->chunk_sync, 0, sizeof(SharedMemory::INFO::SyncObjectChunk));
        ::memcpy(pimpl->info->chunk_sync.chunkId,  chunkId_sync, 4 * sizeof(char));
        pimpl->info->chunk_sync.chunkSize = sizeof(SharedMemory::INFO::SyncObjectChunk);
        GenerateUUIDStringA(pimpl->info->chunk_sync.m_sct_name, SHRD_OBJ_NAME_MAX);
        GenerateUUIDStringA(pimpl->info->chunk_sync.evt_b_name, SHRD_OBJ_NAME_MAX);
        GenerateUUIDStringA(pimpl->info->chunk_sync.evt_r_name, SHRD_OBJ_NAME_MAX);
        GenerateUUIDStringA(pimpl->info->chunk_sync.evt_w_name, SHRD_OBJ_NAME_MAX);

        // 'data' chunk
        ::memcpy(pimpl->info->chunk_data.chunkId,  chunkId_data, 4 * sizeof(char));
        pimpl->info->chunk_data.chunkSize = UINT32_MAX;
    }
    console_out(TEXT("RF64 chunk size: %d"), pimpl->info->chunk_rf64.chunkSize);
    console_out(TEXT("ds64 chunk size: %u"), pimpl->info->chunk_ds64.chunkSize);
    console_out(TEXT("riff size:       %u"), pimpl->info->chunk_ds64.riffSize);
    console_out(TEXT("data size:       %u"), pimpl->info->chunk_ds64.dataSize);
    console_out(TEXT("sync chunk size: %d"), pimpl->info->chunk_sync.chunkSize);
    console_out(TEXT("data chunk size: %d"), pimpl->info->chunk_data.chunkSize);

    // イベントオブジェクトおよび同期オブジェクトを生成
    pimpl->msection  = ::CreateMeteredSectionA(max_reader_count, max_reader_count, pimpl->info->chunk_sync.m_sct_name);
    pimpl->evt_block = ::CreateEventA(nullptr, FALSE, TRUE, pimpl->info->chunk_sync.evt_b_name);
    pimpl->evt_read  = ::CreateEventA(nullptr, FALSE, TRUE, pimpl->info->chunk_sync.evt_r_name);
    pimpl->evt_write = ::CreateEventA(nullptr, FALSE, TRUE, pimpl->info->chunk_sync.evt_w_name);

    console_outA("M_SECTION: %s @ %p",  pimpl->info->chunk_sync.m_sct_name, pimpl->msection);
    console_outA("EVT_BLOCK: %s @ %p",  pimpl->info->chunk_sync.evt_b_name, pimpl->evt_block);
    console_outA("EVT_READ:  %s @ %p",  pimpl->info->chunk_sync.evt_r_name, pimpl->evt_read);
    console_outA("EVT_WRITE: %s @ %p",  pimpl->info->chunk_sync.evt_w_name, pimpl->evt_write);

    if ( nullptr == pimpl->msection )
    {
        console_out(TEXT("CreateMeteredSectionA() failed: %d"), ::GetLastError());
        goto CLOSE;
    }
    if ( nullptr == pimpl->evt_read )
    {
        console_out(TEXT("CreateEventA(evt_read) failed"));
        goto CLOSE;
    }
    if ( nullptr == pimpl->evt_write )
    {
        console_out(TEXT("CreateEventA(evt_write) failed"));
        goto CLOSE;
    }

    // 完了
    console_out(TEXT("SharedMemory::Create() end"));

    return true;

CLOSE: this->Close();

    // 失敗
    console_out(TEXT("SharedMemory::Create() end"));

    return false;
}

//---------------------------------------------------------------------------//

// 既存の共有メモリを開く
bool __stdcall SharedMemory::Open
(
    LPCWSTR name
)
{
    console_out(TEXT("SharedMemory::Open() begin"));

    if ( this->is_open() )
    {
        this->Close();
    }

    // 名前をメンバ変数にコピー
    ::StringCchCopyW(pimpl->name, MAX_PATH, name);
    console_outW(L"name: %s", pimpl->name);

    // マッピングオブジェクトを開く
    pimpl->handle = ::OpenFileMappingW
    (
        FILE_MAP_ALL_ACCESS, FALSE, pimpl->name
    );
    if ( nullptr == pimpl->handle )
    {
        console_out(TEXT("OpenFileMappingW() failed"));
        goto CLOSE;
    }

    // 共有メモリを自プロセスにマップ
    pimpl->info = (SharedMemory::INFO*)::MapViewOfFile
    (
        pimpl->handle, FILE_MAP_ALL_ACCESS, 0, 0, 0
    );
    if ( nullptr == pimpl->info )
    {
        console_out(TEXT("MapViewOfFile() failed"));
        goto CLOSE;
    }
    console_out(TEXT("RF64 chunk size: %d"), pimpl->info->chunk_rf64.chunkSize);
    console_out(TEXT("ds64 chunk size: %u"), pimpl->info->chunk_ds64.chunkSize);
    console_out(TEXT("riff size:       %u"), pimpl->info->chunk_ds64.riffSize);
    console_out(TEXT("data size:       %u"), pimpl->info->chunk_ds64.dataSize);
    console_out(TEXT("sync chunk size: %d"), pimpl->info->chunk_sync.chunkSize);
    console_out(TEXT("data chunk size: %d"), pimpl->info->chunk_data.chunkSize);

    // データ部分の先頭アドレスを取得
    pimpl->data = (uint8_t*)pimpl->info + sizeof(SharedMemory::INFO);

    // イベントオブジェクトおよび同期オブジェクトを取得
    pimpl->msection  = ::OpenMeteredSectionA(pimpl->info->chunk_sync.m_sct_name);
    pimpl->evt_block = ::OpenEventA(EVENT_ALL_ACCESS, FALSE, pimpl->info->chunk_sync.evt_b_name);
    pimpl->evt_read  = ::OpenEventA(EVENT_ALL_ACCESS, FALSE, pimpl->info->chunk_sync.evt_r_name);
    pimpl->evt_write = ::OpenEventA(EVENT_ALL_ACCESS, FALSE, pimpl->info->chunk_sync.evt_w_name);

    console_outA("M_SECTION: %s @ %p",  pimpl->info->chunk_sync.m_sct_name, pimpl->msection);
    console_outA("EVT_BLOCK  %s @ %p",  pimpl->info->chunk_sync.evt_b_name, pimpl->evt_block);
    console_outA("EVT_READ:  %s @ %p",  pimpl->info->chunk_sync.evt_r_name, pimpl->evt_read);
    console_outA("EVT_WRITE  %s @ %p",  pimpl->info->chunk_sync.evt_w_name, pimpl->evt_write);

    if ( nullptr == pimpl->msection )
    {
        console_out(TEXT("OpenMeteredSectionA() failed: %d"), ::GetLastError());
        goto CLOSE;
    }
    if ( nullptr == pimpl->evt_read )
    {
        console_out(TEXT("OpenEventA(evt_read) failed"));
        goto CLOSE;
    }
    if ( nullptr == pimpl->evt_write )
    {
        console_out(TEXT("OpenEventA(evt_write) failed"));
        goto CLOSE;
    }

    // 完了
    console_out(TEXT("SharedMemory::Open() end"));

    return true;

CLOSE: this->Close();

    // 失敗
    console_out(TEXT("SharedMemory::Open() end"));

    return false;
}

//---------------------------------------------------------------------------//

// 共有メモリを閉じる
bool __stdcall SharedMemory::Close()
{
    console_out(TEXT("SharedMemory::Close() begin"));

    if ( ! this->is_open() )
    {
        console_out(TEXT("Already closed"));
        console_out(TEXT("SharedMemory::Close() end"));
        return false;
    }

    // 所有オブジェクトの解放
    if ( pimpl->evt_write )
    {
        ::CloseHandle(pimpl->evt_write);
        pimpl->evt_write = nullptr;
    }
    if ( pimpl->evt_read )
    {
        ::CloseHandle(pimpl->evt_read);
        pimpl->evt_read = nullptr;
    }
    if ( pimpl->evt_block )
    {
        ::CloseHandle(pimpl->evt_block);
        pimpl->evt_block = nullptr;
    }
    if ( pimpl->msection )
    {
        ::CloseMeteredSection(pimpl->msection);
        pimpl->msection = nullptr;
    }
    if ( pimpl->info )
    {
        ::UnmapViewOfFile(pimpl->info);
        pimpl->info = nullptr;
    }
    if ( pimpl->handle )
    {
        ::CloseHandle(pimpl->handle);
        pimpl->handle = nullptr;
    }

    // 名前をリセット
    pimpl->name[0] = L'\0';

    console_out(TEXT("SharedMemory::Close() end"));

    return true;
}

//---------------------------------------------------------------------------//

// 共有メモリのデータを読み込む
size_t __stdcall SharedMemory::Read
(
    void*  buffer,
    size_t offset,
    size_t size,
    DWORD  dwMilliseconds
)
{
    console_out(TEXT("SharedMemory::Read() begin"));

    if ( ! this->is_open() )
    {
        console_out(TEXT("Not opened yet"));
        console_out(TEXT("SharedMemory::Read() end"));
        return 0;
    }
    if ( nullptr == buffer )
    {
        console_out(TEXT("Invalid args: 0x%p"), buffer);
        console_out(TEXT("SharedMemory::Read() end"));
        return 0;
    }

    // 読み込みサイズを計算する
    size_t cb_read;
    #if defined(_WIN64) || defined(WIN64)
        cb_read = min(size, pimpl->info->chunk_ds64.dataSize - offset);
    #else
        uint64_t size64 = min(size, pimpl->info->chunk_ds64.dataSize - offset);
        if ( size64 > UINT32_MAX )
        {
            console_out(TEXT("The size is too large: %u"), size64);
            console_out(TEXT("SharedMemory::Read() end"));
            return 0;
        }
        cb_read = static_cast<size_t>(size64);
    #endif

    DWORD ret;

    // 読み込み開始処理
    console_out(TEXT("Waiting for finish writing..."));
    ret = ::WaitForSingleObject(pimpl->evt_block, dwMilliseconds);
    if ( ret == WAIT_TIMEOUT )
    {
        console_out(TEXT("WaitObject: WAIT_TIMEOUT"));
        console_out(TEXT("SharedMemory::Read() end"));
        return 0;
    }

/// ここから読み込み開始 & 書き込み開始処理はブロックされる

    // 読み込み処理
    ret = ::EnterMeteredSection(pimpl->msection, dwMilliseconds);
    if ( ret == WAIT_TIMEOUT )
    {
        console_out(TEXT("MeteredSection: WAIT_TIMEOUT"));
        console_out(TEXT("SharedMemory::Read() end"));
        return 0;
    }
    console_out(TEXT("Start reading ... lAvailableCount: %d"), pimpl->msection->lpSharedInfo->lAvailableCount);

/// 資源が枯渇した場合、ここから読み込み処理はブロックされる

/// ここから書き込み処理はブロックされる

    ::SetEvent(pimpl->evt_block);

/// ここまで読み込み開始 & 書き込み開始処理はブロックされる

    // 未読み込み状態にする
    ::ResetEvent(pimpl->evt_read);

    ::memcpy(buffer, pimpl->data + offset, cb_read);
    console_out(TEXT("Read %u bytes"), cb_read);

    ::LeaveMeteredSection(pimpl->msection, 1, nullptr);
    console_out(TEXT("Finish reading ... lAvailableCount: %d"), pimpl->msection->lpSharedInfo->lAvailableCount);

/// ここまで書き込み処理はブロックされる

/// 資源が枯渇していた場合、ここまで読み込み処理はブロックされる
    
    // 読み込み完了を通知
    this->NotifyRead();

    console_out(TEXT("SharedMemory::Read() end"));

    return cb_read;
}

//---------------------------------------------------------------------------//

// 共有メモリにデータを書き込む
size_t __stdcall SharedMemory::Write
(
    void*  buffer,
    size_t offset,
    size_t size,
    DWORD  dwMilliseconds
)
{
    console_out(TEXT("SharedMemory::Write() begin"));

    if ( ! this->is_open() )
    {
        console_out(TEXT("Not opened yet"));
        console_out(TEXT("SharedMemory::Write() end"));
        return 0;
    }
    if ( nullptr == buffer )
    {
        console_out(TEXT("Invalid args: 0x%p"), buffer);
        console_out(TEXT("SharedMemory::Write() end"));
        return 0;
    }

    // 書き込みサイズを計算する
    size_t cb_written;
    #if defined(_WIN64) || defined(WIN64)
        cb_written = min(size, pimpl->info->chunk_ds64.dataSize - offset);
    #else
        uint64_t size64 = min(size, pimpl->info->chunk_ds64.dataSize - offset);
        if ( size64 > UINT32_MAX )
        {
            console_out(TEXT("The size is too large: %u"), size64);
            console_out(TEXT("SharedMemory::Read() end"));
            return 0;
        }
        cb_written = static_cast<size_t>(size64);
    #endif

    DWORD ret;

    // 書き込み開始処理
    console_out(TEXT("Waiting for another thread to finish writing..."));
    ret = ::WaitForSingleObject(pimpl->evt_block, dwMilliseconds);
    if ( ret == WAIT_TIMEOUT )
    {
        console_out(TEXT("WaitObject: WAIT_TIMEOUT"));
        console_out(TEXT("SharedMemory::Write() end"));
        return 0;
    }

/// ここから読み込み開始処理はブロックされる

/// ここから書き込み開始処理はブロックされる

/// ここから書き込み処理はブロックされる

    // 読み込み資源の最大数を取得
    const auto max_reader_count = pimpl->msection->lpSharedInfo->lMaximumCount;
    //console_out(TEXT("max_count: %d"), max_count);

    // 全ての読み込み処理が終わるまで待つ
    console_out(TEXT("Waiting for all the threads to finish reading..."));
    for ( LONG count = 0; count < max_reader_count; ++count )
    {
        //console_out(TEXT("%d"), max_reader_count - count);
        ret = ::EnterMeteredSection(pimpl->msection, dwMilliseconds);
        if ( ret == WAIT_TIMEOUT )
        {
            ::LeaveMeteredSection(pimpl->msection, count, nullptr);
            console_out(TEXT("MeteredSection: WAIT_TIMEOUT"));
            console_out(TEXT("SharedMemory::Write() end"));
            return 0;
        }
    }
    console_out(TEXT("Start writing ... lAvailableCount: %d"), pimpl->msection->lpSharedInfo->lAvailableCount);

/// ここから読み込み処理はブロックされる

    // 未書き込み状態にする
    ::ResetEvent(pimpl->evt_write);

    ::memcpy(pimpl->data + offset, buffer, cb_written);
    console_out(TEXT("Wrote %u bytes"), cb_written);

    // 未読み込み状態にする
    ::ResetEvent(pimpl->evt_read);

/// ここまで読み込み処理はブロックされる

/// ここまで書き込み処理はブロックされる

/// ここまで書き込み開始処理はブロックされる

    ::LeaveMeteredSection(pimpl->msection, max_reader_count, nullptr);
    console_out(TEXT("Finish writing ... lAvailableCount: %d"), pimpl->msection->lpSharedInfo->lAvailableCount);

    ::SetEvent(pimpl->evt_block);

/// ここまで読み込み開始処理はブロックされる
    
    // 書き込み完了を通知
    this->NotifyWrite();

    console_out(TEXT("SharedMemory::Write() end"));

    return cb_written;
}

//---------------------------------------------------------------------------//

// 読み込み完了を通知する
void __stdcall SharedMemory::NotifyRead()
{
    ::SetEvent(pimpl->evt_read);
}

//---------------------------------------------------------------------------//

// 書き込み完了を通知する
void __stdcall SharedMemory::NotifyWrite()
{
    ::SetEvent(pimpl->evt_write);
}

//---------------------------------------------------------------------------//

// 読み込み完了を待つ
DWORD __stdcall SharedMemory::WaitRead(DWORD dwMilliseconds)
{
    return ::WaitForSingleObject(pimpl->evt_read, dwMilliseconds);
}

//---------------------------------------------------------------------------//

// 書き込み完了を待つ
DWORD __stdcall SharedMemory::WaitWrite(DWORD dwMilliseconds)
{
    return ::WaitForSingleObject(pimpl->evt_write, dwMilliseconds);
}

//---------------------------------------------------------------------------//

// SharedMemory.cpp