// UWnd.cpp

//---------------------------------------------------------------------------//
//
// ウィンドウの基底クラス
//   Copyright (C) 2005-2013 tapetums
//
//---------------------------------------------------------------------------//

#include "DebugPrint.hpp"

#include "UWnd.hpp"

//---------------------------------------------------------------------------//

static void __stdcall ShowLastError(LPCTSTR mbx_title)
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
    ::MessageBox(nullptr, lpMsgBuf, mbx_title, MB_OK);
    ::LocalFree(lpMsgBuf);
}

//---------------------------------------------------------------------------//

UWnd::UWnd()
{
    console_out(TEXT("UWnd::ctor begin"));

    m_className = TEXT("UWnd");
    UWnd::Register(m_className);

    console_out(TEXT("UWnd::ctor end"));
}

//---------------------------------------------------------------------------//

UWnd::~UWnd()
{
    console_out(TEXT("UWnd::dtor begin"));

    this->Destroy();

    console_out(TEXT("UWnd::dtor end"));
}

//---------------------------------------------------------------------------//

INT32 __stdcall UWnd::X() const
{
    return m_x;
}

//---------------------------------------------------------------------------//

INT32 __stdcall UWnd::Y() const
{
    return m_y;
}

//---------------------------------------------------------------------------//

INT32 __stdcall UWnd::Width() const
{
    return m_w;
}

//---------------------------------------------------------------------------//

INT32 __stdcall UWnd::Height() const
{
    return m_h;
}

//---------------------------------------------------------------------------//

DWORD __stdcall UWnd::Style() const
{
    return (DWORD)::GetWindowLongPtr(m_hwnd, GWL_STYLE);
}

//---------------------------------------------------------------------------//

DWORD __stdcall UWnd::StyleEx() const
{
    return (DWORD)::GetWindowLongPtr(m_hwnd, GWL_EXSTYLE);
}

//---------------------------------------------------------------------------//

HWND __stdcall UWnd::Handle() const
{
    return m_hwnd;
}

//---------------------------------------------------------------------------//

HWND __stdcall UWnd::Parent() const
{
    return (HWND)::GetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT);
}

//---------------------------------------------------------------------------//

LPCTSTR __stdcall UWnd::ClassName() const
{
    return m_className;
}

//---------------------------------------------------------------------------//

bool __stdcall UWnd::IsFullScreen() const
{
    return m_fullscreen;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Create
(
    LPCTSTR lpWindowName,
    DWORD   style, 
    DWORD   styleEx,
    HWND    hwndParent,
    HMENU   hMenu
)
{
    console_out(TEXT("UWnd::Create( %s ) begin"), lpWindowName);

    // 二重生成防止!
    if ( m_hwnd )
    {
        console_out(TEXT("Already created"));
        console_out(TEXT("UWnd::Create(%s) end"), lpWindowName);
        return S_FALSE;
    }

    // 親ウィンドウを持つ場合はウィンドウスタイルにWS_CHILDを追加
    style |= hwndParent ? WS_CHILD : 0;

    // ウィンドウを生成 … UWnd::StaticWndProc() 内で m_hwnd を格納している
    ::CreateWindowEx
    (
        styleEx, m_className, lpWindowName, style,
        m_x, m_y, m_w, m_h,
        hwndParent, hMenu, ::GetModuleHandle(nullptr), (LPVOID)this
    );
    if ( nullptr == m_hwnd )
    {
        // エラーメッセージの表示
        ShowLastError(m_className);
        console_out(TEXT("UWnd::Create(%s) end"), lpWindowName);
        return E_FAIL;
    }

    ::UpdateWindow(m_hwnd);

    console_out(TEXT("UWnd::Create(%s) end"), lpWindowName);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Destroy()
{
    console_out(TEXT("UWnd::Destroy() begin"));

    if ( ::IsWindow(m_hwnd) == FALSE )
    {
        console_out(TEXT("Already destroyed"));
        console_out(TEXT("UWnd::Destroy() end"));
        m_hwnd = nullptr;
        return S_FALSE;
    }

    ::DestroyWindow(m_hwnd);
    m_hwnd = nullptr;

    console_out(TEXT("UWnd::Destroy() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Close()
{
    auto ret = ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);

    return (ret == 0) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Bounds(INT32 x, INT32 y, INT32 w, INT32 h)
{
    this->AdjustRect(w, h);

    auto ret = ::SetWindowPos
    (
        m_hwnd, nullptr,
        x, y, w, h,
        SWP_NOZORDER | SWP_FRAMECHANGED
    );

    return (ret == TRUE) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Hide()
{
    auto ret = ::ShowWindowAsync(m_hwnd, SW_HIDE);

    return (ret == TRUE) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Move(INT32 x, INT32 y)
{
    auto ret = ::SetWindowPos
    (
        m_hwnd, nullptr,
        x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
    );

    return (ret == TRUE) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Refresh(BOOL bErase)
{
    auto ret = ::InvalidateRect(m_hwnd, nullptr, bErase);

    return (ret == TRUE) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Resize(INT32 w, INT32 h)
{
    this->AdjustRect(w, h);

    auto ret = ::SetWindowPos
    (
        m_hwnd, nullptr,
        0, 0, w, h,
        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
    );

    return (ret == TRUE) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::Show()
{
    auto ret = ::ShowWindowAsync(m_hwnd, SW_SHOWNORMAL);
    //::UpdateWindow(m_hwnd);

    return (ret == TRUE) ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

struct MonitorUnderCursor
{
    INT32 x, y, witdh, height;
    WCHAR name[32];

    MonitorUnderCursor()
    {
        console_out(TEXT("Checking which monitor is under the cursor begin"));

        POINT pt;
        ::GetCursorPos(&pt);
        auto hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

        MONITORINFOEX miex = { };
        miex.cbSize = sizeof(miex);
        ::GetMonitorInfo(hMonitor, &miex);
        ::CopyMemory(name, miex.szDevice, 32 * sizeof(WCHAR));

        x      = miex.rcMonitor.left;
        y      = miex.rcMonitor.top;
        witdh  = miex.rcMonitor.right  - miex.rcMonitor.left;
        height = miex.rcMonitor.bottom - miex.rcMonitor.top;

        console_out(TEXT("%s: (X, Y) = (%d, %d)"), name, x, y);
        console_out(TEXT("%s: (Witdh, Height) = (%d, %d)"), name, witdh, height);

        console_out(TEXT("Checking which monitor is under the cursor end"));
    }
};

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::ToCenter()
{
    console_out(TEXT("UWnd::ToCenter() begin"));

    if ( this->Parent() )
    {
        console_out(TEXT("This window has parent ... operation canceled"));
        console_out(TEXT("UWnd::ToCenter() end"));
        return S_FALSE;
    }

    MonitorUnderCursor moniter;
    auto x = (moniter.witdh  - m_w) / 2 + moniter.x;
    auto y = (moniter.height - m_h) / 2 + moniter.y;

    auto result = this->Move(x, y);

    console_out(TEXT("UWnd::ToCenter() end"));

    return result;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UWnd::ToggleFullScreen(INT32 w, INT32 h)
{
    console_out(TEXT("UWnd::ToggleFullScreen() begin"));

    m_fullscreen = !m_fullscreen;
    console_out(TEXT("ToggleFullScreen: %s"), m_fullscreen ? TEXT("true") : TEXT("false"));

    MonitorUnderCursor moniter;

    HRESULT hr;
    if ( m_fullscreen )
    {
        auto style = this->Style() | WS_POPUP | WS_MINIMIZEBOX;
        ::SetWindowLongPtr(m_hwnd, GWL_STYLE, (LONG_PTR)style);

        DEVMODE dm = { };
        dm.dmSize       = sizeof(dm);
        dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
        dm.dmPelsWidth  = moniter.witdh;
        dm.dmPelsHeight = moniter.height;

        ::SetWindowPos
        (
            m_hwnd, nullptr,
            moniter.x, moniter.y, moniter.witdh, moniter.height,
            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );

        hr =  S_OK;
    }
    else
    {
        auto style = this->Style() ^ WS_POPUP;
        ::SetWindowLongPtr(m_hwnd, GWL_STYLE, (LONG_PTR)style);

        this->AdjustRect(w, h);

        ::SetWindowPos
        (
            m_hwnd, nullptr,
            (moniter.witdh  - w) / 2 + moniter.x,
            (moniter.height - h) / 2 + moniter.y,
            w, h,
            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );

        hr =  S_FALSE;
    }

    console_out(TEXT("UWnd::ToggleFullScreen() end"));

    return hr;
}

//---------------------------------------------------------------------------//

LRESULT __stdcall UWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    if ( uMsg == WM_CLOSE )
    {
        console_out(TEXT("UWnd::WndProc( WM_CLOSE ) begin"));
        {
            ::PostQuitMessage(0);
        }
        console_out(TEXT("UWnd::WndProc( WM_CLOSE ) end"));
    }

    return ::DefWindowProc(hwnd, uMsg, wp, lp);
}

//---------------------------------------------------------------------------//

void __stdcall UWnd::Register(LPCTSTR lpszClassName)
{
    // ウィンドウクラスを登録
    WNDCLASSEX wc = { };
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = UWnd::StaticWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ::GetModuleHandle(nullptr);
    wc.hIcon         = nullptr;
    wc.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName  = nullptr;
    wc.lpszClassName = lpszClassName;
    wc.hIconSm       = nullptr;

    auto atom = ::RegisterClassEx(&wc);
    if ( atom == 0 )
    {
        if ( GetLastError() == 0x582 )
        {
            // そのクラスは既にあります。
            return;
        }
        else
        {
            // エラーメッセージの表示
            ShowLastError(lpszClassName);
        }
    }
}

//---------------------------------------------------------------------------//

LRESULT __stdcall UWnd::StaticWndProc
(
    HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp
)
{
    UWnd* wnd = nullptr;

    // UWndオブジェクトのポインタを取得
    if ( uMsg == WM_NCCREATE )
    {
        wnd = (UWnd*)((CREATESTRUCT*)lp)->lpCreateParams;

        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
    }
    else
    {
        wnd = (UWnd*)::GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }

    // ウィンドウプロシージャの呼び出し
    if ( nullptr == wnd )
    {
        console_out(TEXT("Call DefWindowProc(0x%04x)"), uMsg);
        return ::DefWindowProc(hwnd, uMsg, wp, lp);
    }
    else
    {
        // メンバ変数に情報を保存
        switch ( uMsg )
        {
            case WM_CREATE:
            {
                wnd->m_hwnd = hwnd;    // ウィンドウハンドル
                console_out(TEXT("Window Handle: 0x%p"), hwnd);
                break;
            }
            case WM_MOVE:
            {
                wnd->m_x = LOWORD(lp); // ウィンドウx座標
                wnd->m_y = HIWORD(lp); // ウィンドウy座標
                console_out(TEXT("(X, Y) = (%d, %d)"), wnd->m_x, wnd->m_y);
                break;
            }
            case WM_SIZE:
            {
                wnd->m_w = LOWORD(lp); // ウィンドウ幅
                wnd->m_h = HIWORD(lp); // ウィンドウ高
                console_out(TEXT("(Width, Height) = (%d, %d)"), wnd->m_w, wnd->m_h);
                break;
            }
            default:
            {
                break;
            }
        }
        return wnd->WndProc(hwnd, uMsg, wp, lp);
    }
}

//---------------------------------------------------------------------------//

void __stdcall UWnd::AdjustRect(INT32& w, INT32& h) const
{
    console_out(TEXT("UWnd::AdjustRect begin"));
    console_out(TEXT("(w, h) = (%d, %d)"), w, h);

    RECT rc = { 0, 0, w, h };
    BOOL  hasMenu = ::GetMenu(m_hwnd) ? TRUE : FALSE;
    DWORD style   = this->Style();
    DWORD styleEx = this->StyleEx();

    ::AdjustWindowRectEx(&rc, style, hasMenu, styleEx);
    w = rc.right  - rc.left;
    h = rc.bottom - rc.top;

    console_out(TEXT("(w, h) = (%d, %d)"), w, h);
    console_out(TEXT("UWnd::AdjustRect end"));
}

//---------------------------------------------------------------------------//

// UWnd.cpp