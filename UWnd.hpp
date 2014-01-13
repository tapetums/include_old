// UWnd.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// ウィンドウをカプセル化するクラス
//   Copyright (C) 2005-2013 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//

// ウィンドウの基底クラス
class UWnd
{
public:
    explicit UWnd(LPCTSTR lpszClassName);
    virtual ~UWnd();

public:
    UWnd(UWnd&& rhs);
    UWnd& operator =(UWnd&& rhs);

public:
    INT32   __stdcall x()             const;
    INT32   __stdcall y()             const;
    INT32   __stdcall width()         const;
    INT32   __stdcall height()        const;
    DWORD   __stdcall style()         const;
    DWORD   __stdcall styleEx()       const;
    HWND    __stdcall handle()        const;
    HWND    __stdcall parent()        const;
    LPCTSTR __stdcall classname()     const;
    bool    __stdcall is_fullscreen() const;

    virtual HRESULT __stdcall Create
    (
        LPCTSTR lpWindowName = nullptr,
        DWORD   style        = WS_OVERLAPPEDWINDOW, 
        DWORD   styleEx      = 0,
        HWND    hwndParent   = nullptr,
        HMENU   hMenu        = nullptr
    );
    virtual HRESULT __stdcall Destroy();
    virtual HRESULT __stdcall Close();
    virtual HRESULT __stdcall Bounds(INT32 x, INT32 y, INT32 w, INT32 h);
    virtual HRESULT __stdcall Hide();
    virtual HRESULT __stdcall Move(INT32 x, INT32 y);
    virtual HRESULT __stdcall Refresh(BOOL bErase = FALSE);
    virtual HRESULT __stdcall Resize(INT32 w, INT32 h);
    virtual HRESULT __stdcall Show();
    virtual HRESULT __stdcall ToCenter();
    virtual HRESULT __stdcall ToggleFullScreen();
    virtual LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);

private:
    void __stdcall Register() const;
    void __stdcall AdjustRect(INT32& w, INT32& h) const;

protected:
    INT32   m_x             = CW_USEDEFAULT;
    INT32   m_y             = CW_USEDEFAULT;
    INT32   m_w             = CW_USEDEFAULT;
    INT32   m_h             = CW_USEDEFAULT;
    HWND    m_hwnd          = nullptr;
    LPCTSTR m_classname     = nullptr;
    bool    m_is_fullscreen = false;
    DWORD   m_style_old     = 0;
    RECT    m_win_rect;

private:
    UWnd()                        = delete;
    UWnd(const UWnd&)             = delete;
    UWnd& operator= (const UWnd&) = delete;
};

//---------------------------------------------------------------------------//

// AeroGlass対応 ウィンドウの基底クラス
class AeroWnd : public UWnd
{
public:
    explicit AeroWnd(LPCTSTR lpszClassName);
    ~AeroWnd();

public:
    bool __stdcall is_enable_aero() const;

public:
    virtual LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) override;

public:
    virtual void __stdcall EnableAero();
    virtual void __stdcall DisableAero();

protected:
    virtual LRESULT __stdcall OnDwmCompositionChanged();
    virtual LRESULT __stdcall OnThemeChanged();

protected:
    bool   m_enabled     = true;
    HANDLE m_hTheme      = nullptr;
    BOOL   m_compEnabled = FALSE;

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

// OpenGL で描画するウィンドウの基底クラス
class OpenGLWnd : public AeroWnd
{
public:
    explicit OpenGLWnd(LPCTSTR lpszClassName);
    ~OpenGLWnd();

public:
    virtual void __stdcall Update() = 0;

protected:
    virtual void __stdcall CreateContext();
    virtual void __stdcall ReleaseContext();

protected:
    HDC   m_dc   = nullptr;
    HGLRC m_glrc = nullptr;
};

//---------------------------------------------------------------------------//

// UWnd.hpp