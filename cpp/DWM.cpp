// DWM.cpp

//---------------------------------------------------------------------------//
//
// Desktop Window Manager API の ラッパークラス
//   Copyright (C) 2007-2013 tapetums
//
//---------------------------------------------------------------------------//

#include "DWM.hpp"

//---------------------------------------------------------------------------//

#ifdef THIS
#undef THIS
#endif

#define THIS DWM

//---------------------------------------------------------------------------//

struct DWMHolder
{
    HMODULE module;

    DWMHolder()
    {
        module = ::LoadLibraryEx
        (
            TEXT("dwmapi.dll"), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
        );
    }

    ~DWMHolder()
    {
        if ( module )
        {
            ::FreeLibrary(module);
            module = nullptr;
        }
    }
};

//---------------------------------------------------------------------------//

DWM::THIS()
{
    static DWMHolder holder;

    if ( holder.module )
    {
        DwmIsCompositionEnabled = (HRESULT (__stdcall*)(BOOL*))::GetProcAddress
        (
            holder.module, "DwmIsCompositionEnabled"
        );
        DwmEnableBlurBehindWindow = (HRESULT (__stdcall*)(HWND, const DWM_BLURBEHIND*))::GetProcAddress
        (
            holder.module, "DwmEnableBlurBehindWindow"
        );
        DwmExtendFrameIntoClientArea = (HRESULT (__stdcall*)(HWND, const MARGINS*))::GetProcAddress
        (
            holder.module, "DwmExtendFrameIntoClientArea"
        );
    }
}

//---------------------------------------------------------------------------//

bool __stdcall DWM::IsAvailable() const
{
    return ( DwmIsCompositionEnabled &&
             DwmEnableBlurBehindWindow &&
             DwmExtendFrameIntoClientArea ) ?
             true : false;
}

//---------------------------------------------------------------------------//

#undef THIS

//---------------------------------------------------------------------------//

// DWM.cpp