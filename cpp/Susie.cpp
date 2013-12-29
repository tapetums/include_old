// Susie.cpp

//----------------------------------------------------------------------------//
//
// Susie プラグインのラッパークラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <strsafe.h>

#include "DebugPrint.hpp"
#include "UString.hpp"

#include "Susie.hpp"

//---------------------------------------------------------------------------//

Susie::Susie()
{
    console_out(TEXT("Susie::ctor() begin"));

    m_spi_path[0] = '\0';

    console_out(TEXT("Susie::ctor() end"));
}

//---------------------------------------------------------------------------//

Susie::~Susie()
{
    console_out(TEXT("Susie::dtor() begin"));

    this->Free();

    console_out(TEXT("Susie::dtor() end"));
}

//---------------------------------------------------------------------------//

Susie::Susie(Susie&& rhs)
{
    console_out(TEXT("Susie::ctor(move) begin"));

    ::StringCchCopy(m_spi_path, MAX_PATH, rhs.m_spi_path);
    m_module           = rhs.m_module;
    m_GetPluginInfoA   = rhs.m_GetPluginInfoA;
    m_GetPluginInfoW   = rhs.m_GetPluginInfoW;
    m_IsSupportedA     = rhs.m_IsSupportedA;
    m_IsSupportedW     = rhs.m_IsSupportedW;
    m_GetPictureInfoA  = rhs.m_GetPictureInfoA;
    m_GetPictureInfoW  = rhs.m_GetPictureInfoW;
    m_GetPictureA      = rhs.m_GetPictureA;
    m_GetPictureW      = rhs.m_GetPictureW;
    m_GetPreviewA      = rhs.m_GetPreviewA;
    m_GetPreviewW      = rhs.m_GetPreviewW;
    m_GetArchiveInfoA  = rhs.m_GetArchiveInfoA;
    m_GetArchiveInfoW  = rhs.m_GetArchiveInfoW;
    m_GetFileInfoA     = rhs.m_GetFileInfoA;
    m_GetFileInfoW     = rhs.m_GetFileInfoW;
    m_GetFileA         = rhs.m_GetFileA;
    m_GetFileW         = rhs.m_GetFileW;
    m_ConfigurationDlg = rhs.m_ConfigurationDlg;

    rhs.m_spi_path[0]      = '\0';
    rhs.m_module           = nullptr;
    rhs.m_GetPluginInfoA   = nullptr;
    rhs.m_GetPluginInfoW   = nullptr;
    rhs.m_IsSupportedA     = nullptr;
    rhs.m_IsSupportedW     = nullptr;
    rhs.m_GetPictureInfoA  = nullptr;
    rhs.m_GetPictureInfoW  = nullptr;
    rhs.m_GetPictureA      = nullptr;
    rhs.m_GetPictureW      = nullptr;
    rhs.m_GetPreviewA      = nullptr;
    rhs.m_GetPreviewW      = nullptr;
    rhs.m_GetArchiveInfoA  = nullptr;
    rhs.m_GetArchiveInfoW  = nullptr;
    rhs.m_GetFileInfoA     = nullptr;
    rhs.m_GetFileInfoW     = nullptr;
    rhs.m_GetFileA         = nullptr;
    rhs.m_GetFileW         = nullptr;
    rhs.m_ConfigurationDlg = nullptr;

    console_out(TEXT("Susie::ctor(move) end"));
}

//---------------------------------------------------------------------------//

Susie& Susie::operator= (Susie&& rhs)
{
    console_out(TEXT("Susie::operator=(move) begin"));

    ::StringCchCopy(m_spi_path, MAX_PATH, rhs.m_spi_path);
    m_module           = rhs.m_module;
    m_GetPluginInfoA   = rhs.m_GetPluginInfoA;
    m_GetPluginInfoW   = rhs.m_GetPluginInfoW;
    m_IsSupportedA     = rhs.m_IsSupportedA;
    m_IsSupportedW     = rhs.m_IsSupportedW;
    m_GetPictureInfoA  = rhs.m_GetPictureInfoA;
    m_GetPictureInfoW  = rhs.m_GetPictureInfoW;
    m_GetPictureA      = rhs.m_GetPictureA;
    m_GetPictureW      = rhs.m_GetPictureW;
    m_GetPreviewA      = rhs.m_GetPreviewA;
    m_GetPreviewW      = rhs.m_GetPreviewW;
    m_GetArchiveInfoA  = rhs.m_GetArchiveInfoA;
    m_GetArchiveInfoW  = rhs.m_GetArchiveInfoW;
    m_GetFileInfoA     = rhs.m_GetFileInfoA;
    m_GetFileInfoW     = rhs.m_GetFileInfoW;
    m_GetFileA         = rhs.m_GetFileA;
    m_GetFileW         = rhs.m_GetFileW;
    m_ConfigurationDlg = rhs.m_ConfigurationDlg;

    rhs.m_spi_path[0]      = '\0';
    rhs.m_module           = nullptr;
    rhs.m_GetPluginInfoA   = nullptr;
    rhs.m_GetPluginInfoW   = nullptr;
    rhs.m_IsSupportedA     = nullptr;
    rhs.m_IsSupportedW     = nullptr;
    rhs.m_GetPictureInfoA  = nullptr;
    rhs.m_GetPictureInfoW  = nullptr;
    rhs.m_GetPictureA      = nullptr;
    rhs.m_GetPictureW      = nullptr;
    rhs.m_GetPreviewA      = nullptr;
    rhs.m_GetPreviewW      = nullptr;
    rhs.m_GetArchiveInfoA  = nullptr;
    rhs.m_GetArchiveInfoW  = nullptr;
    rhs.m_GetFileInfoA     = nullptr;
    rhs.m_GetFileInfoW     = nullptr;
    rhs.m_GetFileA         = nullptr;
    rhs.m_GetFileW         = nullptr;
    rhs.m_ConfigurationDlg = nullptr;

    console_out(TEXT("Susie::operator=(move) end"));

    return *this;
}

//---------------------------------------------------------------------------//

bool __stdcall Susie::Load(LPCTSTR spi_path)
{
    if ( nullptr == spi_path )
    {
        return false;
    }

    console_out(TEXT("Susie::Load() begin"));

    if ( m_module )
    {
        console_out(TEXT("Already loaded"));
        console_out(TEXT("Susie::Load() end"));
        return false;
    }

    // DLLの読み込み
    m_module = ::LoadLibraryEx
    (
        spi_path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );
    if ( nullptr == m_module )
    {
        console_out(TEXT("LoadLibraryEx() failed"));
        goto FREE_SPI;
    }

    // DLLのフルパスを取得
    ::GetModuleFileName(m_module, m_spi_path, MAX_PATH);
    console_out(m_spi_path);

    // GetPluginInfo
    m_GetPluginInfoA = (SPI_GetPluginInfoA)::GetProcAddress
    (
        m_module, "GetPluginInfo"
    );
    m_GetPluginInfoW = (SPI_GetPluginInfoW)::GetProcAddress
    (
        m_module, "GetPluginInfoW"
    );
    if ( nullptr == m_GetPluginInfoA && nullptr == m_GetPluginInfoW )
    {
        console_out(TEXT("GetPluginInfo was not found"));
        goto FREE_SPI;
    }
    // IsSupported
    m_IsSupportedA = (SPI_IsSupportedA)::GetProcAddress
    (
        m_module, "IsSupported"
    );
    m_IsSupportedW = (SPI_IsSupportedW)::GetProcAddress
    (
        m_module, "IsSupportedW"
    );
    if ( nullptr == m_IsSupportedA && nullptr == m_IsSupportedW )
    {
        console_out(TEXT("IsSupported was not found"));
        goto FREE_SPI;
    }

    // GetPictureInfo
    m_GetPictureInfoA = (SPI_GetPictureInfoA)::GetProcAddress
    (
        m_module, "GetPictureInfo"
    );
    m_GetPictureInfoW = (SPI_GetPictureInfoW)::GetProcAddress
    (
        m_module, "GetPictureInfoW"
    );
    // GetPicture
    m_GetPictureA = (SPI_GetPictureA)::GetProcAddress
    (
        m_module, "GetPicture"
    );
    m_GetPictureW = (SPI_GetPictureW)::GetProcAddress
    (
        m_module, "GetPictureW"
    );
    // GetPreview
    m_GetPreviewA = (SPI_GetPreviewA)::GetProcAddress
    (
        m_module, "GetPreview"
    );
    m_GetPreviewW = (SPI_GetPreviewW)::GetProcAddress
    (
        m_module, "GetPreviewW"
    );

    // GetArchiveInfo
    m_GetArchiveInfoA = (SPI_GetArchiveInfoA)::GetProcAddress
    (
        m_module, "GetArchiveInfo"
    );
    m_GetArchiveInfoW = (SPI_GetArchiveInfoW)::GetProcAddress
    (
        m_module, "GetArchiveInfoW"
    );
    // GetFileInfo
    m_GetFileInfoA = (SPI_GetFileInfoA)::GetProcAddress
    (
        m_module, "GetFileInfo"
    );
    m_GetFileInfoW = (SPI_GetFileInfoW)::GetProcAddress
    (
        m_module, "GetFileInfoW"
    );
    // GetFile
    m_GetFileA = (SPI_GetFileA)::GetProcAddress
    (
        m_module, "GetFile"
    );
    m_GetFileW = (SPI_GetFileW)::GetProcAddress
    (
        m_module, "GetFileW"
    );

    // ConfigurationDlg
    m_ConfigurationDlg = (SPI_ConfigurationDlg)::GetProcAddress
    (
        m_module, "ConfigurationDlg"
    );

    console_out(TEXT("Loaded"));
    console_out(TEXT("Susie::Load() end"));

    return true;

FREE_SPI: this->Free();

    console_out(TEXT("Susie::Load() end"));

    return false;
}

//---------------------------------------------------------------------------//

bool __stdcall Susie::Free()
{
    console_out(TEXT("Susie::Free() begin"));
    console_out(m_spi_path);

    if ( m_module )
    {
        ::FreeLibrary(m_module);
    }

    m_spi_path[0]      = '\0';
    m_module           = nullptr;
    m_GetPluginInfoA   = nullptr;
    m_GetPluginInfoW   = nullptr;
    m_IsSupportedA     = nullptr;
    m_IsSupportedW     = nullptr;
    m_GetPictureInfoA  = nullptr;
    m_GetPictureInfoW  = nullptr;
    m_GetPictureA      = nullptr;
    m_GetPictureW      = nullptr;
    m_GetPreviewA      = nullptr;
    m_GetPreviewW      = nullptr;
    m_GetArchiveInfoA  = nullptr;
    m_GetArchiveInfoW  = nullptr;
    m_GetFileInfoA     = nullptr;
    m_GetFileInfoW     = nullptr;
    m_GetFileA         = nullptr;
    m_GetFileW         = nullptr;
    m_ConfigurationDlg = nullptr;

    console_out(TEXT("Freed"));
    console_out(TEXT("Susie::Free() end"));

    return true;
}

//---------------------------------------------------------------------------//

LPCTSTR __stdcall Susie::SpiPath() const
{
    return m_spi_path;
}

//---------------------------------------------------------------------------//

#if defined(_UNICODE) || defined(UNICODE)

//---------------------------------------------------------------------------//

// マルチバイト文字列をワイド文字列に変換して
// 同じバッファ内に無理矢理つっこむ関数
void __stdcall MBCSztoUnicodez(char* buf, size_t buflen)
{
    const auto tmp = toUnicodez(buf);

    ::memcpy(buf, tmp, buflen);
    buf[buflen - 1] = '\0';
}

//---------------------------------------------------------------------------//

// ワイド文字列をマルチバイト文字列に変換して
// 同じバッファ内に無理矢理つっこむ関数
void __stdcall UnicodeztoMBCSz(wchar_t* buf)
{
    const auto tmp = toMBCSz(buf);
    const auto buflen = sizeof(wchar_t) * (::wcslen(buf) + 1);
    const auto tmplen = sizeof(char)    * (::strlen(tmp) + 1);

    ::memset(buf, 0, buflen);
    ::memcpy(buf, tmp, min(buflen, tmplen));
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPluginInfo(int32_t infono, LPWSTR buf, int32_t buflen)
{
    if ( m_GetPluginInfoW )
    {
        return m_GetPluginInfoW(infono, buf, buflen);
    }
    else if ( m_GetPluginInfoA )
    {
        const auto result = m_GetPluginInfoA(infono, (LPSTR)buf, buflen);
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
    if ( m_IsSupportedW )
    {
        return m_IsSupportedW(filename, dw);
    }
    else if ( m_IsSupportedA )
    {
        UnicodeztoMBCSz((LPWSTR)filename);

        return m_IsSupportedA((LPCSTR)filename, dw);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPictureInfo(LPCWSTR buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo)
{
    if ( m_GetPictureInfoW )
    {
        return m_GetPictureInfoW(buf, len, flag, lpInfo);
    }
    else if ( m_GetPictureInfoA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return m_GetPictureInfoA((LPCSTR)buf, len, flag, lpInfo);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPicture(LPCWSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( m_GetPictureW )
    {
        return m_GetPictureW(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else if ( m_GetPictureA )
    {
    
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return m_GetPictureA((LPCSTR)buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPreview(LPCWSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( m_GetPreviewW )
    {
        return m_GetPreviewW(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else if ( m_GetPreviewA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return m_GetPreviewA((LPCSTR)buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetArchiveInfo(LPCWSTR buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf)
{
    if ( m_GetArchiveInfoW )
    {
        return m_GetArchiveInfoW(buf, len, flag, lphInf);
    }
    else if ( m_GetArchiveInfoA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }
        return m_GetArchiveInfoA((LPCSTR)buf, len, flag, lphInf);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFileInfo(LPCWSTR buf, size_t len, LPCWSTR filename, SPI_FLAG flag, SusieFileInfoW* lpInfo)
{
    if ( m_GetFileInfoW )
    {
        return m_GetFileInfoW(buf, len, filename, flag, lpInfo);
    }
    else if ( m_GetFileInfoA )
    {
        UnicodeztoMBCSz((LPWSTR)filename);
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)buf);
        }

        SusieFileInfoA info;
        const auto result =  m_GetFileInfoA((LPCSTR)buf, len, (LPCSTR)filename, flag, &info);
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
    if ( m_GetFileW )
    {
        return m_GetFileW(src, len, dest, flag, progressCallback, lData);
    }
    else if ( m_GetFileA )
    {
        if ( !(flag & SPI_INPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)src);
        }
        if ( !(flag & SPI_OUTPUT_MEMORY) )
        {
            UnicodeztoMBCSz((LPWSTR)dest);
        }
        return m_GetFileA((LPCSTR)src, len, (LPSTR)dest, flag, progressCallback, lData);
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
    if ( m_GetPluginInfoA )
    {
        return m_GetPluginInfoA(infono, buf, buflen);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::IsSupported(LPCSTR filename, void* dw)
{
    if ( m_IsSupportedA )
    {
        return m_IsSupportedA(filename, dw);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPictureInfo(LPCSTR buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo)
{
    if ( m_GetPictureInfoA )
    {
        return m_GetPictureInfoA(buf, len, flag, lpInfo);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPicture(LPCSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( m_GetPictureA )
    {
        return m_GetPictureA(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetPreview(LPCSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( m_GetPreviewA )
    {
        return m_GetPreviewA(buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetArchiveInfo(LPCSTR buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf)
{
    if ( m_GetArchiveInfoA )
    {
        return m_GetArchiveInfoA(buf, len, flag, lphInf);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFileInfo(LPCSTR buf, size_t len, LPCSTR filename, SPI_FLAG flag, SusieFileInfoA* lpInfo)
{
    if ( m_GetFileInfoA )
    {
        return m_GetFileInfoA(buf, len, filename, flag, lpInfo);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

SUSIE_API Susie::GetFile(LPCSTR src, size_t len, LPSTR dest, SPI_FLAG flag, SUSIE_PROGRESS progressCallback, intptr_t lData)
{
    if ( m_GetFileA )
    {
        return m_GetFileA(src, len, dest, flag, progressCallback, lData);
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
    if ( m_ConfigurationDlg )
    {
        return m_ConfigurationDlg(parent, fnc);
    }
    else
    {
        return SPI_NO_FUNCTION;
    }
}

//---------------------------------------------------------------------------//

// Susie.cpp
