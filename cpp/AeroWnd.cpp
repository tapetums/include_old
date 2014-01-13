// AeroWnd.cpp

//---------------------------------------------------------------------------//
//
// AeroGlass対応 ウィンドウの基底クラス
//   Copyright (C) 2005-2013 tapetums
//
//---------------------------------------------------------------------------//

#include "DebugPrint.hpp"

#include "DWM.hpp"
#include "UxTheme.hpp"

#include "UWnd.hpp"

//---------------------------------------------------------------------------//

struct AeroWnd::Impl
{
    DWM     dwm;
    UxTheme uxtheme;
};

//---------------------------------------------------------------------------//
//
// Ctor / Dtor
//
//---------------------------------------------------------------------------//

AeroWnd::AeroWnd(LPCTSTR lpszClassName) : UWnd(lpszClassName)
{
    console_out(TEXT("AeroWnd::ctor begin"));

    pimpl = new Impl;

    if ( pimpl->uxtheme.IsAvailable() )
    {
        pimpl->uxtheme.BufferedPaintInit();
    }

    console_out(TEXT("AeroWnd::ctor end"));
}

//---------------------------------------------------------------------------//

AeroWnd::~AeroWnd()
{
    console_out(TEXT("AeroWnd::dtor begin"));

    if ( pimpl->uxtheme.IsAvailable() )
    {
        if ( m_hTheme )
        {
            pimpl->uxtheme.CloseThemeData(m_hTheme);
            m_hTheme = nullptr;
        }
        pimpl->uxtheme.BufferedPaintUnInit();
    }

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("AeroWnd::dtor end"));
}

//---------------------------------------------------------------------------//
//
// Properties
//
//---------------------------------------------------------------------------//

bool __stdcall AeroWnd::is_enable_aero() const
{
    return m_enabled;
}

//---------------------------------------------------------------------------//
//
// Window Procedure to be Overriden
//
//---------------------------------------------------------------------------//

LRESULT __stdcall AeroWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            return this->OnThemeChanged();
        }
        case WM_DWMCOMPOSITIONCHANGED:
        {
            return this->OnDwmCompositionChanged();
        }
        case WM_THEMECHANGED:
        {
            return this->OnThemeChanged();
        }
        default:
        {
            return __super::WndProc(hwnd, uMsg, wp, lp);
        }
    }
}

//---------------------------------------------------------------------------//
//
// Methids
//
//---------------------------------------------------------------------------//

void __stdcall AeroWnd::EnableAero()
{
    if ( ! pimpl->dwm.IsAvailable() )
    {
        return;
    }

    pimpl->dwm.DwmIsCompositionEnabled(&m_compEnabled);
    if ( m_compEnabled )
    {
        DWM_BLURBEHIND bb = { };
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = TRUE;
        bb.hRgnBlur = nullptr;
        pimpl->dwm.DwmEnableBlurBehindWindow(m_hwnd, &bb);

        MARGINS margins = { -1 };
        pimpl->dwm.DwmExtendFrameIntoClientArea(m_hwnd, &margins);
    }

    ::InvalidateRect(m_hwnd, nullptr, TRUE);

    m_enabled = true;
}

//---------------------------------------------------------------------------//

void __stdcall AeroWnd::DisableAero()
{
    if ( ! pimpl->dwm.IsAvailable() )
    {
        return;
    }

    pimpl->dwm.DwmIsCompositionEnabled(&m_compEnabled);
    if ( m_compEnabled )
    {
        DWM_BLURBEHIND bb = { };
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = FALSE;
        bb.hRgnBlur = nullptr;
        pimpl->dwm.DwmEnableBlurBehindWindow(m_hwnd, &bb);

        MARGINS margins = { };
        pimpl->dwm.DwmExtendFrameIntoClientArea(m_hwnd, &margins);
    }

    ::InvalidateRect(m_hwnd, nullptr, TRUE);

    m_enabled = false;
}

//---------------------------------------------------------------------------//
//
// Event Handlers
//
//---------------------------------------------------------------------------//

LRESULT __stdcall AeroWnd::OnDwmCompositionChanged()
{
    if ( m_enabled ) this->EnableAero();

    return 0L;
}

//---------------------------------------------------------------------------//

LRESULT __stdcall AeroWnd::OnThemeChanged()
{
    if ( pimpl->uxtheme.IsAvailable() )
    {
        if ( m_hTheme )
        {
            pimpl->uxtheme.CloseThemeData(m_hTheme);
            m_hTheme = nullptr;
        }
        if ( pimpl->uxtheme.IsThemeActive() )
        {
            m_hTheme = pimpl->uxtheme.OpenThemeData(m_hwnd, L"BUTTON");
        }
    }

    return this->OnDwmCompositionChanged();
}

//---------------------------------------------------------------------------//

// AeroWnd.cpp