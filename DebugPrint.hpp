// DebugPrint.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// デバッグウィンドウへの出力関数
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#if defined(_DEBUG) || defined(DEBUG)

// Debugのとき
void __stdcall console_outA(const char*    format, ...);
void __stdcall console_outW(const wchar_t* format, ...);

#if defined(_UNICODE) || defined(UNICODE)
    #define console_out console_outW
#else
    #define console_out console_outA
#endif

#else

// Releaseのとき
#define console_out(x, ...)
#define console_outA(x, ...)
#define console_outW(x, ...)

#endif

//---------------------------------------------------------------------------//

// DebugPrint.hpp