// DebugPrint.cpp

//---------------------------------------------------------------------------//
//
// デバッグウィンドウへの出力関数
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#if defined(_DEBUG) || defined(DEBUG)

#include <windows.h>
#include <strsafe.h>

#include "DebugPrint.hpp"

//---------------------------------------------------------------------------//

#define thread_local __declspec(thread) static

//---------------------------------------------------------------------------//

#ifndef _tcsstr
#if defined(_UNICODE) || defined(UNICODE)
    #define _tcsstr wcsstr
#else
    #define _tcsstr strstr
#endif
#endif

//---------------------------------------------------------------------------//

#define BUFSIZE 1024
#define white FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY

//---------------------------------------------------------------------------//

thread_local size_t     indent    = 0;
thread_local DWORD      thread_id = 0;
thread_local WORD       color     = 0;
thread_local SYSTEMTIME st        = { };

//---------------------------------------------------------------------------//

class ConsoleHolder
{
public:
    HANDLE evt  = nullptr;
    HANDLE hout = nullptr;

public:
    static ConsoleHolder* __stdcall GetInstance()
    {
        static ConsoleHolder holder;
        return &holder;
    }

    void __stdcall EnterSection()
    {
        ::WaitForSingleObject(evt, INFINITE);
    }

    void __stdcall LeaveSection()
    {
        ::SetEvent(evt);
    }

private:
    ConsoleHolder()
    {
        evt = ::CreateEventW(nullptr, FALSE, TRUE, L"console");
        if ( nullptr == evt )
        {
            evt = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, L"console");
        }

        if ( FALSE == ::AttachConsole(ATTACH_PARENT_PROCESS) )
        {
            ::AllocConsole();
        }

        hout = ::GetStdHandle(STD_OUTPUT_HANDLE);
    }

    ~ConsoleHolder()
    {
        if ( evt )
        {
            ::CloseHandle(evt);
            evt = nullptr;
        }
        if ( hout )
        {
            //::FreeConsole();
            hout = nullptr;
        }
    }
};

//---------------------------------------------------------------------------//

static inline void __stdcall get_params()
{
    ::GetLocalTime(&st);

    if ( thread_id == 0 )
    {
        thread_id = ::GetCurrentThreadId();

        color = 0x0F & (thread_id >> 4);
        if ( color == 0 )
        {
            color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    }
}

//---------------------------------------------------------------------------//

void __stdcall console_outA(const char* format, ...)
{
    // コンソールウィンドウのインスタンスを取得
    auto console = ConsoleHolder::GetInstance();

    DWORD cb_w;
    thread_local char spaces[BUFSIZE];
    thread_local char pre[BUFSIZE];
    thread_local char buf[BUFSIZE];

    // インデントを下げる
    if ( strstr(format, " end") )
    {
        --indent;
    }

    // インデントを文字列に
    size_t i = 0;
    for ( ; i < indent; ++i )
    {
        spaces[i*2]     = '.';
        spaces[i*2 + 1] = ' ';
    }
    spaces[i*2] = '\0';

    // 各種情報を取得
    get_params();

    // 時刻を文字列に
    ::StringCchPrintfA
    (
        pre, BUFSIZE,
        "0x%04x %02u:%02u:%02u;%03u> %s",
        thread_id,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        spaces
    );

    // 引数を文字列に
    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfA(buf, BUFSIZE, format, al);
    }
    va_end(al);

    // 文字列の書き出し
    console->EnterSection();
    {
        ::SetConsoleTextAttribute(console->hout, color);
        ::WriteConsoleA(console->hout, "*", 1, &cb_w, nullptr);

        ::SetConsoleTextAttribute(console->hout, white);
        ::WriteConsoleA(console->hout, pre, ::lstrlenA(pre), &cb_w, nullptr);
        ::WriteConsoleA(console->hout, buf, ::lstrlenA(buf), &cb_w, nullptr);
        ::WriteConsoleA(console->hout, "\n", 1, &cb_w, nullptr);
    }
    console->LeaveSection();

    // インデントを上げる
    if ( strstr(format, " begin") )
    {
        ++indent;
    }
}

//---------------------------------------------------------------------------//

void __stdcall console_outW(const wchar_t* format, ...)
{
    // コンソールウィンドウのインスタンスを取得
    auto console = ConsoleHolder::GetInstance();

    DWORD cb_w;
    thread_local wchar_t spaces[BUFSIZE];
    thread_local wchar_t pre[BUFSIZE];
    thread_local wchar_t buf[BUFSIZE];

    // インデントを下げる
    if ( wcsstr(format, L" end") )
    {
        --indent;
    }

    // インデントを文字列に
    size_t i = 0;
    for ( ; i < indent; ++i )
    {
        spaces[i*2]     = '.';
        spaces[i*2 + 1] = ' ';
    }
    spaces[i*2] = '\0';

    // 各種情報を取得
    get_params();

    // 時刻を文字列に
    ::StringCchPrintfW
    (
        pre, BUFSIZE,
        L"0x%04x %02u:%02u:%02u;%03u> %s",
        thread_id,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        spaces
    );

    // 引数を文字列に
    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfW(buf, BUFSIZE, format, al);
    }
    va_end(al);

    // 文字列の書き出し
    console->EnterSection();
    {
        ::SetConsoleTextAttribute(console->hout, color);
        ::WriteConsoleW(console->hout, L"*", 1, &cb_w, nullptr);

        ::SetConsoleTextAttribute(console->hout, white);
        ::WriteConsoleW(console->hout, pre, ::lstrlenW(pre), &cb_w, nullptr);
        ::WriteConsoleW(console->hout, buf, ::lstrlenW(buf), &cb_w, nullptr);
        ::WriteConsoleW(console->hout, L"\n", 1, &cb_w, nullptr);
    }
    console->LeaveSection();

    // インデントを上げる
    if ( wcsstr(format, L" begin") )
    {
        ++indent;
    }
}

//---------------------------------------------------------------------------//

#endif // #if defined(_DEBUG) || defined(DEBUG)

//---------------------------------------------------------------------------//

// DebugPrint.cpp