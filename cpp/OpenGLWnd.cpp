// OpenGLWnd.cpp

//---------------------------------------------------------------------------//
//
// OpenGL で描画するウィンドウの基底クラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <Windows.h>
#include <windowsx.h>

#include <gl/gl.h>
#include <gl/glext.h>
#pragma comment(lib, "opengl32.lib")

#include "DebugPrint.hpp"

#include "UWnd.hpp"

//---------------------------------------------------------------------------//

static const PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0, 0, 0, 0,
    24,
    8,
    0,
    PFD_MAIN_PLANE,
    0,
    0, 0, 0
};

//---------------------------------------------------------------------------//

static void __stdcall OutputLastError()
{
    LPTSTR lpMsgBuf = nullptr;
    ::FormatMessage
    (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        nullptr
    );
    console_out(lpMsgBuf);
    ::LocalFree(lpMsgBuf);
}

//---------------------------------------------------------------------------//

OpenGLWnd::OpenGLWnd(LPCTSTR lpszClassName) : AeroWnd(lpszClassName)
{
    console_out(TEXT("OpenGLWnd::ctor begin"));

    console_out(TEXT("OpenGLWnd::ctor end"));
}

//---------------------------------------------------------------------------//

OpenGLWnd::~OpenGLWnd()
{
    console_out(TEXT("OpenGLWnd::dtor begin"));

    console_out(TEXT("OpenGLWnd::dtor end"));
}

//---------------------------------------------------------------------------//

void __stdcall OpenGLWnd::CreateContext()
{
    console_out(TEXT("OpenGLWnd::CreateContext() begin"));

    m_dc = ::GetDC(m_hwnd);

    auto pxfmt = ::ChoosePixelFormat(m_dc, &pfd);
    if ( 0 == pxfmt )
    {
        OutputLastError();
        this->ReleaseContext();
        return;
    }

    auto result = ::SetPixelFormat(m_dc, pxfmt, &pfd);
    if ( FALSE == result )
    {
        OutputLastError();
        this->ReleaseContext();
        return;
    }

    m_glrc = ::wglCreateContext(m_dc);
    ::wglMakeCurrent(m_dc, m_glrc);
    console_out(TEXT("HGLRC: 0x%p"), m_glrc);

    console_out(TEXT("OpenGLWnd::CreateContext() end"));
}

//---------------------------------------------------------------------------//

void __stdcall OpenGLWnd::ReleaseContext()
{
    console_out(TEXT("OpenGLWnd::ReleaseContext() begin"));

    ::wglMakeCurrent(m_dc, nullptr);

    ::wglDeleteContext(m_glrc);
    m_glrc = nullptr;

    ::ReleaseDC(m_hwnd, m_dc);
    m_dc = nullptr;

    console_out(TEXT("OpenGLWnd::ReleaseContext() end"));
}

//---------------------------------------------------------------------------//

// OpenGLWnd.cpp