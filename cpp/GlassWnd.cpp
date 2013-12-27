// GlassWnd.cpp

//---------------------------------------------------------------------------//
//
// AeroGlass対応 ウィンドウの基底クラス
//   Copyright (C) 2005-2013 tapetums
//
//---------------------------------------------------------------------------//

#include "DebugPrint.hpp"

#include "UWnd.hpp"

//---------------------------------------------------------------------------//

#include "DWM.hpp"
#include "UxTheme.hpp"

//---------------------------------------------------------------------------//

#undef  BASE
#define BASE UWnd

//---------------------------------------------------------------------------//

GlassWnd::GlassWnd()
{
    console_out(TEXT("GlassWnd::ctor begin"));

    dwm     = new DWM;
    uxtheme = new UxTheme;

    if ( uxtheme->IsAvailable() )
    {
        uxtheme->BufferedPaintInit();
    }

    console_out(TEXT("GlassWnd::ctor end"));
}

//---------------------------------------------------------------------------//

GlassWnd::~GlassWnd()
{
    console_out(TEXT("GlassWnd::dtor begin"));

    if ( uxtheme->IsAvailable() )
    {
        if ( m_hTheme )
        {
            uxtheme->CloseThemeData(m_hTheme);
            m_hTheme = nullptr;
        }
        uxtheme->BufferedPaintUnInit();
    }

    delete uxtheme;
    uxtheme = nullptr;

    delete dwm;
    dwm = nullptr;

    console_out(TEXT("GlassWnd::dtor end"));
}

//---------------------------------------------------------------------------//

LRESULT __stdcall GlassWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    switch (uMsg)
    {
        case WM_DWMCOMPOSITIONCHANGED:
        {
            return this->OnDwmCompositionChanged(hwnd);
        }
        case WM_THEMECHANGED:
        {
            return this->OnThemeChanged(hwnd);
        }
        default:
        {
            return BASE::WndProc(hwnd, uMsg, wp, lp);
        }
    }
}

//---------------------------------------------------------------------------//

LRESULT __stdcall GlassWnd::OnDwmCompositionChanged(HWND hwnd)
{
    if ( !dwm->IsAvailable() )
    {
        return -1L;
    }

    dwm->DwmIsCompositionEnabled(&m_compEnabled);
    if ( m_compEnabled )
    {
        DWM_BLURBEHIND bb = { };
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = TRUE;
        bb.hRgnBlur = nullptr;
        dwm->DwmEnableBlurBehindWindow(hwnd, &bb);

        MARGINS margins = { -1 };
        dwm->DwmExtendFrameIntoClientArea(hwnd, &margins);
    }

    ::InvalidateRect(hwnd, nullptr, TRUE);

    return 0L;
}

//---------------------------------------------------------------------------//

LRESULT __stdcall GlassWnd::OnThemeChanged(HWND hwnd)
{
    if ( uxtheme->IsAvailable() )
    {
        if ( m_hTheme )
        {
            uxtheme->CloseThemeData(m_hTheme);
            m_hTheme = nullptr;
        }
        if ( uxtheme->IsThemeActive() )
        {
            m_hTheme = uxtheme->OpenThemeData(hwnd, L"BUTTON");
        }
    }

    return this->OnDwmCompositionChanged(hwnd);
}

//---------------------------------------------------------------------------//

#undef BASE

//---------------------------------------------------------------------------//

// GlassWnd.cpp