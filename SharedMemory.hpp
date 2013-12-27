// SharedMemory.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// 共有メモリをカプセル化するクラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include "riff.h"

//---------------------------------------------------------------------------//

#define SHRD_OBJ_NAME_MAX 40

//---------------------------------------------------------------------------//

// 共有メモリをカプセル化するクラス
class SharedMemory
{
public:
#pragma pack(push, 1)
    struct INFO
    {
        struct SyncObjectChunk
        {
            char     chunkId[4];
            uint32_t chunkSize;
            char     m_sct_name[SHRD_OBJ_NAME_MAX];
            char     evt_b_name[SHRD_OBJ_NAME_MAX];
            char     evt_r_name[SHRD_OBJ_NAME_MAX];
            char     evt_w_name[SHRD_OBJ_NAME_MAX];
            uint8_t  user_data [32];
        };

        RF64Chunk       chunk_rf64;
        DataSize64Chunk chunk_ds64;
        SyncObjectChunk chunk_sync;
        DataChunk       chunk_data;
    };
#pragma pack(pop)

public:
    SharedMemory();
    ~SharedMemory();

public:
    SharedMemory(const SharedMemory& lhs);
    SharedMemory(SharedMemory&& rhs);
    SharedMemory& operator =(const SharedMemory& lhs);
    SharedMemory& operator =(SharedMemory&& rhs);

public:
    LPCWSTR  __stdcall name()      const;
    bool     __stdcall is_open()   const;
    INFO*    __stdcall info()      const;
    uint8_t* __stdcall data()      const;
    uint64_t __stdcall data_size() const;
    HANDLE   __stdcall evt_read()  const;
    HANDLE   __stdcall evt_write() const;

public:
    bool   __stdcall Create(LPCWSTR name, uint64_t data_size, LONG max_reader_count = 0);
    bool   __stdcall Open(LPCWSTR name);
    bool   __stdcall Close();
    size_t __stdcall Read (void* buffer, size_t offset, size_t size, DWORD dwMilliseconds = INFINITE);
    size_t __stdcall Write(void* buffer, size_t offset, size_t size, DWORD dwMilliseconds = INFINITE);
    void   __stdcall NotifyRead ();
    void   __stdcall NotifyWrite();
    DWORD  __stdcall WaitRead (DWORD dwMilliseconds = INFINITE);
    DWORD  __stdcall WaitWrite(DWORD dwMilliseconds = INFINITE);

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

// SharedMemory.hpp