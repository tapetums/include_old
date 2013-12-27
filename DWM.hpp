// DWM.hpp

#pragma once

///---------------------------------------------------------------------------//
//
// Desktop Window Manager API の ラッパークラス
//   Copyright (C) 2007-2013 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <dwmapi.h>

//---------------------------------------------------------------------------//

#ifdef THIS
#undef THIS
#endif

#define THIS DWM

// Desktop Window Manager API の ラッパークラス
class DWM
{
public:
    THIS();
    ~THIS() = default;

public:
    bool __stdcall IsAvailable() const;

    HRESULT (__stdcall* DwmIsCompositionEnabled)(BOOL*) = nullptr;
    HRESULT (__stdcall* DwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*) = nullptr;
    HRESULT (__stdcall* DwmExtendFrameIntoClientArea)(HWND, const MARGINS*) = nullptr;

private:
    THIS(const THIS&)             = delete;
    THIS(THIS&&)                  = delete;
    THIS& operator= (const THIS&) = delete;
    THIS& operator= (THIS&&)      = delete;
};

#undef THIS

//---------------------------------------------------------------------------//

// DWM.hpp