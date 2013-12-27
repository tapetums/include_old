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

thread_local size_t indent = 0;

//---------------------------------------------------------------------------//

class ConsoleHolder
{
public:
    HANDLE evt    = nullptr;
    HANDLE hout   = nullptr;

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

        hout = GetStdHandle(STD_OUTPUT_HANDLE);
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
            system("pause");
        }

        ::FreeConsole();
    }
};

//---------------------------------------------------------------------------//

static inline void __stdcall get_params(SYSTEMTIME& st, DWORD& thread_id, WORD& color)
{
    ::GetLocalTime(&st);

    thread_id = ::GetCurrentThreadId();

    color = 0x0F & (thread_id >> 4);
    if ( color == 0 )
    {
        color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

//---------------------------------------------------------------------------//

void __stdcall console_outA(const char* format, ...)
{
    auto console = ConsoleHolder::GetInstance();

    SYSTEMTIME st;
    DWORD      thread_id;
    WORD       color;
    get_params(st, thread_id, color);

    char spaces[BUFSIZE];
    char buf[BUFSIZE];

    console->EnterSection();

    DWORD cb_w = 0;
    if ( strstr(format, " end") )
    {
        if ( indent < 1 )
        {
            auto buf = "!\n";
            ::WriteConsoleA(console->hout, buf, ::lstrlenA(buf), &cb_w, nullptr);
        }
        else
        {
            --indent;
        }
    }

    size_t i = 0;
    for ( ; i < indent; ++i )
    {
        spaces[i*2]     = '.';
        spaces[i*2 + 1] = ' ';
    }
    spaces[i*2] = '\0';

    ::SetConsoleTextAttribute(console->hout, color);
    ::WriteConsoleA(console->hout, "*", 1, &cb_w, nullptr);
    ::SetConsoleTextAttribute(console->hout, white);

    ::StringCchPrintfA
    (
        buf, BUFSIZE,
        "%04u %02d:%02d:%02d;%03d> %s",
        thread_id,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        spaces
    );
    ::WriteConsoleA(console->hout, buf, ::lstrlenA(buf), &cb_w, nullptr);

    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfA(buf, BUFSIZE, format, al);
    }
    va_end(al);
    ::WriteConsoleA(console->hout, buf, ::lstrlenA(buf), &cb_w, nullptr);

    ::WriteConsoleA(console->hout, "\n", 1, &cb_w, nullptr);


    if ( strstr(format, " begin") )
    {
        ++indent;
    }

    console->LeaveSection();
}

//---------------------------------------------------------------------------//

void __stdcall console_outW(const wchar_t* format, ...)
{
    auto console = ConsoleHolder::GetInstance();

    SYSTEMTIME st;
    DWORD      thread_id;
    WORD       color;
    get_params(st, thread_id, color);

    wchar_t spaces[BUFSIZE];
    wchar_t buf[BUFSIZE];

    console->EnterSection();

    DWORD cb_w = 0;
    if ( wcsstr(format, L" end") )
    {
        if ( indent < 1 )
        {
            auto buf = L"!\n";
            ::WriteConsoleW(console->hout, buf, ::lstrlenW(buf), &cb_w, nullptr);
        }
        else
        {
            --indent;
        }
    }

    size_t i = 0;
    for ( ; i < indent; ++i )
    {
        spaces[i*2]     = '.';
        spaces[i*2 + 1] = ' ';
    }
    spaces[i*2] = '\0';

    ::SetConsoleTextAttribute(console->hout, color);
    ::WriteConsoleW(console->hout, L"*", 1, &cb_w, nullptr);
    ::SetConsoleTextAttribute(console->hout, white);

    ::StringCchPrintfW
    (
        buf, BUFSIZE,
        L"%04u %02d:%02d:%02d;%03d> %s",
        thread_id,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        spaces
    );
    ::WriteConsoleW(console->hout, buf, ::lstrlenW(buf), &cb_w, nullptr);

    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfW(buf, BUFSIZE, format, al);
    }
    va_end(al);
    ::WriteConsoleW(console->hout, buf, ::lstrlenW(buf), &cb_w, nullptr);

    ::WriteConsoleW(console->hout, L"\n", 1, &cb_w, nullptr);

    if ( wcsstr(format, L" begin") )
    {
        ++indent;
    }

    console->LeaveSection();
}

//---------------------------------------------------------------------------//

#endif // #if defined(_DEBUG) || defined(DEBUG)

//---------------------------------------------------------------------------//

// DebugPrint.cpp