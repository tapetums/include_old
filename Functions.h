// Functions.h

#pragma once

//---------------------------------------------------------------------------//

extern "C"
{
    typedef HRESULT (__stdcall* DLLCANUNLOADNOW)();
    typedef HRESULT (__stdcall* DLLGETCLASSOBJECT)(REFCLSID, REFIID, void**);
    typedef HRESULT (__stdcall* DLLGETPROPERTY)(size_t, void**);
    typedef HRESULT (__stdcall* DLLOPENCONFIGURATION)(HWND, HWND*);
}

//---------------------------------------------------------------------------//

// Functions.h