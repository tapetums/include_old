// Susie.cpp

//----------------------------------------------------------------------------//
//
// Susie プラグインのラッパークラス
//   Copyright (C) 2013-2014 tapetums
//
//---------------------------------------------------------------------------//

#include "DebugPrint.hpp"
#include "UString.hpp"

#include "Susie.hpp"

//---------------------------------------------------------------------------//
//
// Pimpl Idiom
//
//---------------------------------------------------------------------------//

struct Susie::Impl
{
    TCHAR                path[MAX_PATH];
    HMODULE              module           = nullptr;
    SPI_GetPluginInfoA   GetPluginInfoA   = nullptr;
    SPI_GetPluginInfoW   GetPluginInfoW   = nullptr;
    SPI_IsSupportedA     IsSupportedA     = nullptr;
    SPI_IsSupportedW     IsSupportedW     = nullptr;
    SPI_GetPictureInfoA  GetPictureInfoA  = nullptr;
    SPI_GetPictureInfoW  GetPictureInfoW  = nullptr;
    SPI_GetPictureA      GetPictureA      = nullptr;
    SPI_GetPictureW      GetPictureW      = nullptr;
    SPI_GetPreviewA      GetPreviewA      = nullptr;
    SPI_GetPreviewW      GetPreviewW      = nullptr;
    SPI_GetArchiveInfoA  GetArchiveInfoA  = nullptr;
    SPI_GetArchiveInfoW  GetArchiveInfoW  = nullptr;
    SPI_GetFileInfoA     GetFileInfoA     = nullptr;
    SPI_GetFileInfoW     GetFileInfoW     = nullptr;
    SPI_GetFileA         GetFileA         = nullptr;
    SPI_GetFileW         GetFileW         = nullptr;
    SPI_ConfigurationDlg ConfigurationDlg = nullptr;
};

//---------------------------------------------------------------------------//
//
// Ctor / Dtor
//
//---------------------------------------------------------------------------//

Susie::Susie(LPCTSTR path)
{
    console_out(TEXT("%s::ctor() begin"), TEXT(__FILE__));

    pimpl = new Impl;
    pimpl->path[0] = '\0';

    this->Load(path);

    console_out(TEXT("%s::ctor() end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

Susie::~Susie()
{
    console_out(TEXT("%s::dtor() begin"), TEXT(__FILE__));

    if ( nullptr == pimpl )
    {
        console_out(TEXT("Already moved"));
    }
    else
    {
        this->Free();

        delete pimpl;
        pimpl = nullptr;
    }

    console_out(TEXT("%s::dtor() end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

Susie::Susie(Susie&& rhs)
{
    console_out(TEXT("%s::ctor(move) begin"), TEXT(__FILE__));

    rhs.pimpl = pimpl;

    pimpl = nullptr;

    console_out(TEXT("%s::ctor(move) end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

Susie& Susie::operator= (Susie&& rhs)
{
    console_out(TEXT("%s::operator=(move) begin"), TEXT(__FILE__));

    rhs.pimpl = pimpl;

    pimpl = nullptr;

    console_out(TEXT("%s::operator=(move) end"), TEXT(__FILE__));

    return *this;
}

//---------------------------------------------------------------------------//
//
// Properties
//
//---------------------------------------------------------------------------//

LPCTSTR __stdcall Susie::path() const
{
    return pimpl->path;
}

//---------------------------------------------------------------------------//
//
// Methods
//
//---------------------------------------------------------------------------//

HRESULT __stdcall Susie::Load(LPCTSTR path)
{
    if ( nullptr == path )
    {
        return E_INVALIDARG;
    }

    console_out(TEXT("%s::Load() begin"), TEXT(__FILE__));

    if ( pimpl->module )
    {
        console_out(TEXT("Already loaded"));
        console_out(TEXT("%s::Load() end"), TEXT(__FILE__));
        return S_FALSE;
    }

    // DLLの読み込み
    pimpl->module = ::LoadLibraryEx
    (
        path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );
    if ( nullptr == pimpl->module )
    {
        console_out(TEXT("LoadLibraryEx() failed"));
        goto FREE_SPI;
    }

    // DLLのフルパスを取得
    ::GetModuleFileName(pimpl->module, pimpl->path, MAX_PATH);
    console_out(pimpl->path);

    // GetPluginInfo
    pimpl->GetPluginInfoA = (SPI_GetPluginInfoA)::GetProcAddress
    (
        pimpl->module, "GetPluginInfo"
    );
    pimpl->GetPluginInfoW = (SPI_GetPluginInfoW)::GetProcAddress
    (
        pimpl->module, "GetPluginInfoW"
    );
    if ( nullptr == pimpl->GetPluginInfoA && nullptr == pimpl->GetPluginInfoW )
    {
        console_out(TEXT("GetPluginInfo was not found"));
        goto FREE_SPI;
    }
    // IsSupported
    pimpl->IsSupportedA = (SPI_IsSupportedA)::GetProcAddress
    (
        pimpl->module, "IsSupported"
    );
    pimpl->IsSupportedW = (SPI_IsSupportedW)::GetProcAddress
    (
        pimpl->module, "IsSupportedW"
    );
    if ( nullptr == pimpl->IsSupportedA && nullptr == pimpl->IsSupportedW )
    {
        console_out(TEXT("IsSupported was not found"));
        goto FREE_SPI;
    }

    // GetPictureInfo
    pimpl->GetPictureInfoA = (SPI_GetPictureInfoA)::GetProcAddress
    (
        pimpl->module, "GetPictureInfo"
    );
    pimpl->GetPictureInfoW = (SPI_GetPictureInfoW)::GetProcAddress
    (
        pimpl->module, "GetPictureInfoW"
    );
    // GetPicture
    pimpl->GetPictureA = (SPI_GetPictureA)::GetProcAddress
    (
        pimpl->module, "GetPicture"
    );
    pimpl->GetPictureW = (SPI_GetPictureW)::GetProcAddress
    (
        pimpl->module, "GetPictureW"
    );
    // GetPreview
    pimpl->GetPreviewA = (SPI_GetPreviewA)::GetProcAddress
    (
        pimpl->module, "GetPreview"
    );
    pimpl->GetPreviewW = (SPI_GetPreviewW)::GetProcAddress
    (
        pimpl->module, "GetPreviewW"
    );

    // GetArchiveInfo
    pimpl->GetArchiveInfoA = (SPI_GetArchiveInfoA)::GetProcAddress
    (
        pimpl->module, "GetArchiveInfo"
    );
    pimpl->GetArchiveInfoW = (SPI_GetArchiveInfoW)::GetProcAddress
    (
        pimpl->module, "GetArchiveInfoW"
    );
    // GetFileInfo
    pimpl->GetFileInfoA = (SPI_GetFileInfoA)::GetProcAddress
    (
        pimpl->module, "GetFileInfo"
    );
    pimpl->GetFileInfoW = (SPI_GetFileInfoW)::GetProcAddress
    (
        pimpl->module, "GetFileInfoW"
    );
    // GetFile
    pimpl->GetFileA = (SPI_GetFileA)::GetProcAddress
    (
        pimpl->module, "GetFile"
    );
    pimpl->GetFileW = (SPI_GetFileW)::GetProcAddress
    (
        pimpl->module, "GetFileW"
    );

    // ConfigurationDlg
    pimpl->ConfigurationDlg = (SPI_ConfigurationDlg)::GetProcAddress
    (
        pimpl->module, "ConfigurationDlg"
    );

    console_out(TEXT("Loaded"));
    console_out(TEXT("%s::Load() end"), TEXT(__FILE__));

    return S_OK;

FREE_SPI: this->Free();

    console_out(TEXT("%s::Load() end"), TEXT(__FILE__));

    return E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Susie::Free()
{
    console_out(TEXT("%s::Free() begin"), TEXT(__FILE__));
    console_out(pimpl->path);

    HRESULT hr = S_OK;

    if ( pimpl->module )
    {
        ::FreeLibrary(pimpl->module);
    }
    else
    {
        hr = S_FALSE;
    }

    pimpl->path[0]          = '\0';
    pimpl->module           = nullptr;
    pimpl->GetPluginInfoA   = nullptr;
    pimpl->GetPluginInfoW   = nullptr;
    pimpl->IsSupportedA     = nullptr;
    pimpl->IsSupportedW     = nullptr;
    pimpl->GetPictureInfoA  = nullptr;
    pimpl->GetPictureInfoW  = nullptr;
    pimpl->GetPictureA      = nullptr;
    pimpl->GetPictureW      = nullptr;
    pimpl->GetPreviewA      = nullptr;
    pimpl->GetPreviewW      = nullptr;
    pimpl->GetArchiveInfoA  = nullptr;
    pimpl->GetArchiveInfoW  = nullptr;
    pimpl->GetFileInfoA     = nullptr;
    pimpl->GetFileInfoW     = nullptr;
    pimpl->GetFileA         = nullptr;
    pimpl->GetFileW         = nullptr;
    pimpl->ConfigurationDlg = nullptr;

    console_out(TEXT("Freed"));
    console_out(TEXT("%s::Free() end"), TEXT(__FILE__));

    return hr;
}

//---------------------------------------------------------------------------//

#if defined(_UNICODE) || defined(UNICODE)

//---------------------------------------------------------------------------//
//
// Utility Functions
//
//---------------------------------------------------------------------------//

// マルチバイト文字列をワイド文字列に変換して
// 同じバッファ内に無理矢理つっこむ関数
static void __stdcall MBCSztoUnicodez(char* buf, size_t buflen)
{
    const auto tmp = toUnicodez(buf);

    ::memcpy(buf, tmp, buflen);
    buf[buflen - 1] = '\0';
}

//---------------------------------------------------------------------------//

// ワイド文字列をマルチバイト文字列に変換して
// 同じバッファ内に無理矢理つっこむ関数
static void __stdcall UnicodeztoMBCSz(wchar_t* buf)
{
    const auto tmp = toMBCSz(buf);
    const auto buflen = sizeof(wchar_t) * (::wcslen(buf) + 1);
    const auto tmplen = sizeof(char)    * (::strlen(tmp) + 1);

    ::memset(buf, 0, buflen);
    ::memcpy(buf, tmp, min(buflen, tmplen));
}

//---------------------------------------------------------------------------//
//
// Methods
//
//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPluginInfo(int32_t infono, LPWSTR buf, int32_t buflen)
{
    if ( pimpl->GetPluginInfoW )
    {
        return pimpl->GetPluginInfoW(infono, buf, buflen);
    }
    else if ( pimpl->GetPluginInfoA )
    {
        const auto result = pimpl->GetPluginInfoA(infono, (LPSTR)buf, buflen);
        if ( result == SPI_ALL_RIGHT )
        {
            MBCSztoUnicodez((LPSTR)buf, buflen);
        }
        return result;
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::IsSupported(LPCWSTR filename, void* dw)
{
    if ( pimpl->IsSupportedW )
    {
        return pimpl->IsSupportedW(filename, dw);
    }
    else if ( pimpl->IsSupportedA )
    {
        UnicodeztoMBCSz((LPWSTR)filename);

        return pimpl->IsSupportedA((LPCSTR)filename, dw);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPictureInfo(LPCWSTR buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo)
{
    if ( pimpl->GetPictureInfoW )
    {
        return pimpl->GetPictureInfoW(buf, len, flag, lpInfo);
    }
    else if ( pimpl->GetPictureInfoA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return pimpl->GetPictureInfoA((LPCSTR)buf, len, flag, lpInfo);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPicture(LPCWSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( pimpl->GetPictureW )
    {
        return pimpl->GetPictureW(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else if ( pimpl->GetPictureA )
    {
    
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return pimpl->GetPictureA((LPCSTR)buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPreview(LPCWSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( pimpl->GetPreviewW )
    {
        return pimpl->GetPreviewW(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else if ( pimpl->GetPreviewA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return pimpl->GetPreviewA((LPCSTR)buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetArchiveInfo(LPCWSTR buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf)
{
    if ( pimpl->GetArchiveInfoW )
    {
        return pimpl->GetArchiveInfoW(buf, len, flag, lphInf);
    }
    else if ( pimpl->GetArchiveInfoA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return pimpl->GetArchiveInfoA((LPCSTR)buf, len, flag, lphInf);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFileInfo(LPCWSTR buf, size_t len, LPCWSTR filename, SPI_FLAG flag, SusieFileInfoW* lpInfo)
{
    if ( pimpl->GetFileInfoW )
    {
        return pimpl->GetFileInfoW(buf, len, filename, flag, lpInfo);
    }
    else if ( pimpl->GetFileInfoA )
    {
        UnicodeztoMBCSz((LPWSTR)filename);
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }

        SusieFileInfoA info;
        const auto result =  pimpl->GetFileInfoA((LPCSTR)buf, len, (LPCSTR)filename, flag, &info);
        if ( result == SPI_ALL_RIGHT )
        {
            ::CopyMemory(lpInfo,           &info,          sizeof(uint8_t)*8 + sizeof(size_t)*3 + sizeof(susie_time_t));
            ::CopyMemory(lpInfo->path,     &info.path,     sizeof(char)*SPI_MAX_PATH);
            ::CopyMemory(lpInfo->filename, &info.filename, sizeof(char)*SPI_MAX_PATH);
            lpInfo->crc = info.crc;

            MBCSztoUnicodez((LPSTR)lpInfo->path,     sizeof(wchar_t)*SPI_MAX_PATH);
            MBCSztoUnicodez((LPSTR)lpInfo->filename, sizeof(wchar_t)*MAX_PATH);
        }
        return result;
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFile(LPCWSTR src, size_t len, LPWSTR dest, SPI_FLAG flag, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( pimpl->GetFileW )
    {
        return pimpl->GetFileW(src, len, dest, flag, progressCallback, lData);
    }
    else if ( pimpl->GetFileA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)src);
        }
        if ( !(flag & SPI_OUTPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)dest);
        }
        return pimpl->GetFileA((LPCSTR)src, len, (LPSTR)dest, flag, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

#else

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPluginInfo(int32_t infono, LPSTR buf, int32_t buflen)
{
    if ( pimpl->GetPluginInfoA )
    {
        return pimpl->GetPluginInfoA(infono, buf, buflen);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::IsSupported(LPCSTR filename, void* dw)
{
    if ( pimpl->IsSupportedA )
    {
        return pimpl->IsSupportedA(filename, dw);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPictureInfo(LPCSTR buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo)
{
    if ( pimpl->GetPictureInfoA )
    {
        return pimpl->GetPictureInfoA(buf, len, flag, lpInfo);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPicture(LPCSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( pimpl->GetPictureA )
    {
        return pimpl->GetPictureA(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPreview(LPCSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( pimpl->GetPreviewA )
    {
        return pimpl->GetPreviewA(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetArchiveInfo(LPCSTR buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf)
{
    if ( pimpl->GetArchiveInfoA )
    {
        return pimpl->GetArchiveInfoA(buf, len, flag, lphInf);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFileInfo(LPCSTR buf, size_t len, LPCSTR filename, SPI_FLAG flag, SusieFileInfoA* lpInfo)
{
    if ( pimpl->GetFileInfoA )
    {
        return pimpl->GetFileInfoA(buf, len, filename, flag, lpInfo);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFile(LPCSTR src, size_t len, LPSTR dest, SPI_FLAG flag, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( pimpl->GetFileA )
    {
        return pimpl->GetFileA(src, len, dest, flag, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

#endif // #if defined(_UNICODE) || defined(UNICODE)

//---------------------------------------------------------------------------//

SUSIE_API Susie::ConfigurationDlg(HWND parent, SPI_FNC_CODE fnc)
{
    if ( pimpl->ConfigurationDlg )
    {
        return pimpl->ConfigurationDlg(parent, fnc);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

// Susie.cpp
