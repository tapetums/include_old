// UString.cpp

//---------------------------------------------------------------------------//
//
// 文字コード相互変換ユーティリティ
//   Copyright (C) 2013 tapetums
//
// 131222: thread_local を マクロで定義
//  VC++ に thread_local が実装されるまでのつなぎです。
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "UString.hpp"

//---------------------------------------------------------------------------//

#define BUF_SIZE 1024
#define CP_UTF32 12000

//---------------------------------------------------------------------------//

#define thread_local __declspec(thread) static

//---------------------------------------------------------------------------//

char* __stdcall toMBCSz(const char* s)
{
    return (char*)s;
}

//---------------------------------------------------------------------------//

char* __stdcall toMBCSz(const wchar_t* s)
{
    thread_local char buf[BUF_SIZE];

    ::WideCharToMultiByte
    (
        CP_ACP, 0, s, -1, buf, BUF_SIZE,
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char* __stdcall toMBCSz(const char8_t* s)
{
    thread_local char    buf[BUF_SIZE];
    thread_local wchar_t tmp[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF8, 0, (char*)s, -1, tmp, BUF_SIZE
    );
    ::WideCharToMultiByte
    (
        CP_ACP, 0, tmp, -1, buf, BUF_SIZE,
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char* __stdcall toMBCSz(const char16_t* s)
{
    return toMBCSz((const wchar_t*)s);
}

//---------------------------------------------------------------------------//

char* __stdcall toMBCSz(const char32_t* s)
{
    thread_local char    buf[BUF_SIZE];
    thread_local wchar_t tmp[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF32, 0, (char*)s, -1, tmp, BUF_SIZE
    );
    ::WideCharToMultiByte
    (
        CP_ACP, 0, tmp, -1, buf, BUF_SIZE,
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

wchar_t* __stdcall toUnicodez(const char* s)
{
    thread_local wchar_t buf[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_ACP, 0, s, -1, buf, BUF_SIZE
    );

    return buf;
}

//---------------------------------------------------------------------------//

wchar_t* __stdcall toUnicodez(const wchar_t* s)
{
    return (wchar_t*)s;
}

//---------------------------------------------------------------------------//

wchar_t* __stdcall toUnicodez(const char8_t* s)
{
    thread_local wchar_t buf[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF8, 0, (char*)s, -1, buf, BUF_SIZE
    );

    return buf;
}

//---------------------------------------------------------------------------//

wchar_t* __stdcall toUnicodez(const char16_t* s)
{
    return (wchar_t*)s;
}

//---------------------------------------------------------------------------//

wchar_t* __stdcall toUnicodez(const char32_t* s)
{
    thread_local wchar_t buf[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF32, 0, (char*)s, -1, buf, BUF_SIZE
    );

    return buf;
}

//---------------------------------------------------------------------------//

char8_t* __stdcall toUTF8z(const char* s)
{
    thread_local char8_t buf[BUF_SIZE];
    thread_local wchar_t tmp[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_ACP, 0, s, -1, tmp, BUF_SIZE
    );
    ::WideCharToMultiByte
    (
        CP_UTF8, 0, tmp, -1, (char*)buf, BUF_SIZE * sizeof(char8_t),
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char8_t* __stdcall toUTF8z(const wchar_t* s)
{
    thread_local char8_t buf[BUF_SIZE];

    ::WideCharToMultiByte
    (
        CP_UTF8, 0, s, -1, (char*)buf, BUF_SIZE * sizeof(char8_t),
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char8_t* __stdcall toUTF8z(const char8_t* s)
{
    return (char8_t*)s;
}

//---------------------------------------------------------------------------//

char8_t* __stdcall toUTF8z(const char16_t* s)
{
    return toUTF8z((const wchar_t*)s);
}

//---------------------------------------------------------------------------//

char8_t* __stdcall toUTF8z(const char32_t* s)
{
    thread_local char8_t buf[BUF_SIZE];
    thread_local wchar_t tmp[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF32, 0, (char*)s, -1, tmp, BUF_SIZE
    );
    ::WideCharToMultiByte
    (
        CP_UTF8, 0, tmp, -1, (char*)buf, BUF_SIZE * sizeof(char8_t),
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char16_t* __stdcall toUTF16z(const char* s)
{
    return (char16_t*)toUnicodez(s);
}

//---------------------------------------------------------------------------//

char16_t* __stdcall toUTF16z(const wchar_t* s)
{
    return (char16_t*)s;
}

//---------------------------------------------------------------------------//

char16_t* __stdcall toUTF16z(const char8_t* s)
{
    return (char16_t*)toUnicodez(s);
}

//---------------------------------------------------------------------------//

char16_t* __stdcall toUTF16z(const char16_t* s)
{
    return (char16_t*)s;
}

//---------------------------------------------------------------------------//

char16_t* __stdcall toUTF16z(const char32_t* s)
{
    thread_local char16_t buf[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF32, 0, (char*)s, -1, (wchar_t*)buf, BUF_SIZE
    );

    return buf;
}

//---------------------------------------------------------------------------//

char32_t* __stdcall toUTF32z(const char* s)
{
    thread_local char32_t buf[BUF_SIZE];
    thread_local wchar_t  tmp[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_ACP, 0, s, -1, tmp, BUF_SIZE
    );
    ::WideCharToMultiByte
    (
        CP_UTF32, 0, tmp, -1, (char*)buf, BUF_SIZE * sizeof(char32_t),
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char32_t* __stdcall toUTF32z(const wchar_t* s)
{
    thread_local char32_t buf[BUF_SIZE];

    ::WideCharToMultiByte
    (
        CP_UTF8, 0, s, -1, (char*)buf, BUF_SIZE * sizeof(char32_t),
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char32_t* __stdcall toUTF32z(const char8_t* s)
{
    thread_local char32_t buf[BUF_SIZE];
    thread_local wchar_t  tmp[BUF_SIZE];

    ::MultiByteToWideChar
    (
        CP_UTF8, 0, (char*)s, -1, tmp, BUF_SIZE
    );
    ::WideCharToMultiByte
    (
        CP_UTF32, 0, tmp, -1, (char*)buf, BUF_SIZE * sizeof(char32_t),
        nullptr, nullptr
    );

    return buf;
}

//---------------------------------------------------------------------------//

char32_t* __stdcall toUTF32z(const char16_t* s)
{
    return toUTF32z((const wchar_t*)s);
}

//---------------------------------------------------------------------------//

char32_t* __stdcall toUTF32z(const char32_t* s)
{
    return (char32_t*)s;
}

//---------------------------------------------------------------------------//

// UString.cpp