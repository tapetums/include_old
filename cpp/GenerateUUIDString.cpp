﻿// GenerateUUIDString.cpp

//---------------------------------------------------------------------------//
//
// レジストリ形式の UUID 文字列を生成するユーティリティ関数
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>
#include <strsafe.h>

#include "GenerateUUIDString.hpp"

//---------------------------------------------------------------------------//

void __stdcall GenerateUUIDStringA(char* buf, size_t cch_buf)
{
    if ( cch_buf >= 38 + 1 )
    {
        struct REG_UUID
        {
            uint32_t data1;
            uint16_t data2;
            uint16_t data3;
            uint16_t data4;
            uint8_t  data5[6];
        } uuid;

        ::CoCreateGuid((GUID*)&uuid);

        ::StringCchPrintfA
        (
            buf, cch_buf,
            "{%08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X}",
            uuid.data1, uuid.data2, uuid.data3, uuid.data4,
            uuid.data5[0], uuid.data5[1], uuid.data5[2],
            uuid.data5[3], uuid.data5[4], uuid.data5[5]
        );
    }
    else if ( cch_buf >= 32 + 1 )
    {
        GUID guid;
        ::CoCreateGuid((GUID*)&guid);

        ::StringCchPrintfA
        (
            buf, cch_buf,
            "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
        );
    }
    else
    {
        buf[0] = L'\0';
    }
}

//---------------------------------------------------------------------------//

void __stdcall GenerateUUIDStringW(wchar_t* buf, size_t cch_buf)
{
    if ( cch_buf >= 38 + 1 )
    {
        struct REG_UUID
        {
            uint32_t data1;
            uint16_t data2;
            uint16_t data3;
            uint16_t data4;
            uint8_t  data5[6];
        } uuid;

        ::CoCreateGuid((GUID*)&uuid);

        ::StringCchPrintfW
        (
            buf, cch_buf,
            L"{%08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X}",
            uuid.data1, uuid.data2, uuid.data3, uuid.data4,
            uuid.data5[0], uuid.data5[1], uuid.data5[2],
            uuid.data5[3], uuid.data5[4], uuid.data5[5]
        );
    }
    else if ( cch_buf >= 32 + 1 )
    {
        GUID guid;
        ::CoCreateGuid((GUID*)&guid);

        ::StringCchPrintfW
        (
            buf, cch_buf,
            L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
        );
    }
    else
    {
        buf[0] = L'\0';
    }
}

//---------------------------------------------------------------------------//

// GenerateUUIDString.cpp