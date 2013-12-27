// SpiManager.cpp

//---------------------------------------------------------------------------//
//
// Susie プラグイン管理クラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <vector>

#include <DebugPrint.hpp>

#include "Susie.hpp"

//---------------------------------------------------------------------------//

struct SpiManager::Impl
{
    std::vector<Susie> plugins;
};

//---------------------------------------------------------------------------//

SpiManager::SpiManager()
{
    console_out(TEXT("SpiManager::ctor() begin"));

    pimpl = new Impl;

    console_out(TEXT("SpiManager::ctor() end"));
}

//---------------------------------------------------------------------------//

SpiManager::~SpiManager()
{
    console_out(TEXT("SpiManager::dtor() begin"));

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("SpiManager::dtor() end"));
}

//---------------------------------------------------------------------------//

void __stdcall SpiManager::Append(LPCTSTR spi_path)
{
    console_out(TEXT("SpiManager::Append() begin"));

    Susie susie;

    if ( susie.Load(spi_path) )
    {
        pimpl->plugins.push_back(std::move(susie));
    }

    console_out(TEXT("SpiManager::Append() end"));
}

//---------------------------------------------------------------------------//

void __stdcall SpiManager::Remove(LPCTSTR spi_path)
{
    console_out(TEXT("SpiManager::Remove() begin"));

    auto it  = pimpl->plugins.begin();
    auto end = pimpl->plugins.end();

    while ( it != end )
    {
        if ( lstrcmp((*it).SpiPath(), spi_path) == 0 )
        {
            pimpl->plugins.erase(it);
            break;
        }
        ++it;
    }

    console_out(TEXT("SpiManager::Remove() end"));
}

//---------------------------------------------------------------------------//

size_t __stdcall SpiManager::SpiCount() const
{
    return pimpl->plugins.size();
}

//---------------------------------------------------------------------------//

Susie* __stdcall SpiManager::GetAt(size_t index) const
{
    Susie* susie;

    try
    {
        susie = &pimpl->plugins.at(index);
    }
    catch ( std::out_of_range& )
    {
        //::MessageBoxA(nullptr, e.what(), "SpiManager::GetAt()", MB_OK);
        susie = nullptr;
    }

    return susie;
}

//---------------------------------------------------------------------------//

Susie* __stdcall SpiManager::QueryAvailableSpi(LPCTSTR filename, void* dw) const
{
    Susie* susie = nullptr;

    for ( auto& plugin : pimpl->plugins )
    {
        auto result = plugin.IsSupported(filename, dw);
        if ( result == SPI_SUPPORTED )
        {
            susie = &plugin;
            break;
        }
    }

    return susie;
}

//---------------------------------------------------------------------------//

// SpiManager.cpp