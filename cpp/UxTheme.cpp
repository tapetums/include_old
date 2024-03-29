// UxTheme.cpp

//---------------------------------------------------------------------------//
//
// Visual Style API の ラッパークラス
//   Copyright (C) 2007-2013 tapetums
//
//---------------------------------------------------------------------------//

#include "UxTheme.hpp"

//---------------------------------------------------------------------------//

struct UxThemeHolder
{
    HMODULE module;

    UxThemeHolder()
    {
        module = ::LoadLibraryEx
        (
            TEXT("uxtheme.dll"), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
        );
    }

    ~UxThemeHolder()
    {
        if ( module )
        {
            ::FreeLibrary(module);
            module = nullptr;
        }
    }
};

//---------------------------------------------------------------------------//

#ifdef THIS
#undef THIS
#endif

#define THIS UxTheme

//---------------------------------------------------------------------------//

UxTheme::THIS()
{
    static UxThemeHolder holder;

    if ( holder.module )
    {
        BeginBufferedPaint = (BEGINBUFFEREDPAINT)::GetProcAddress
        (
            holder.module, "BeginBufferedPaint"
        );
        BufferedPaintInit = (BUFFEREDPAINTINIT)::GetProcAddress
        (
            holder.module, "BufferedPaintInit"
        );
        BufferedPaintSetAlpha = (BUFFEREDPAINTSETALPHA)::GetProcAddress
        (
            holder.module, "BufferedPaintSetAlpha"
        );
        BufferedPaintUnInit = (BUFFEREDPAINTUNINIT)::GetProcAddress
        (
            holder.module, "BufferedPaintUnInit"
        );
        CloseThemeData = (CLOSETHEMEDATA)::GetProcAddress
        (
            holder.module, "CloseThemeData"
        );
        DrawThemeBackground = (DRAWTHEMEBACKGROUND)::GetProcAddress
        (
            holder.module, "DrawThemeBackground"
        );
        DrawThemeTextEx = (DRAWTHEMETEXTEX)::GetProcAddress
        (
            holder.module, "DrawThemeTextEx"
        );
        EndBufferedPaint = (ENDBUFFEREDPAINT)::GetProcAddress
        (
            holder.module, "EndBufferedPaint"
        );
        OpenThemeData = (OPENTHEMEDATA)::GetProcAddress
        (
            holder.module, "OpenThemeData"
        );
        IsThemeActive = (ISTHEMEACTIVE)::GetProcAddress
        (
            holder.module, "IsThemeActive"
        );
    }
}

//---------------------------------------------------------------------------//

bool __stdcall UxTheme::IsAvailable() const
{
    return ( BufferedPaintInit && BufferedPaintUnInit ) ? true : false;
}

//---------------------------------------------------------------------------//

#undef THIS

//---------------------------------------------------------------------------//

// UxTheme.cpp