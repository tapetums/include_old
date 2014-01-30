// SusieCollection.cpp

//---------------------------------------------------------------------------//
//
// Susie プラグイン管理クラス
//   Copyright (C) 2013-2014 tapetums
//
//---------------------------------------------------------------------------//

#include <memory>
#include <vector>

#include "DebugPrint.hpp"
#include "CriticalSection.hpp"

#include "Susie.hpp"

//---------------------------------------------------------------------------//
//
// Pimpl Idiom
//
//---------------------------------------------------------------------------//

struct SusieCollection::Impl
{
    CriticalSection cs;
    std::vector<Susie> plugins;
};

//---------------------------------------------------------------------------//
//
// Ctor / Dtor
//
//---------------------------------------------------------------------------//

SusieCollection::SusieCollection()
{
    console_out(TEXT("SusieCollection::ctor() begin"));

    pimpl = new Impl;

    console_out(TEXT("SusieCollection::ctor() end"));
}

//---------------------------------------------------------------------------//

SusieCollection::~SusieCollection()
{
    console_out(TEXT("SusieCollection::dtor() begin"));

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("SusieCollection::dtor() end"));
}

//---------------------------------------------------------------------------//
//
// Properties
//
//---------------------------------------------------------------------------//

size_t __stdcall SusieCollection::size() const
{
    return pimpl->plugins.size();
}

//---------------------------------------------------------------------------//

Susie* __stdcall SusieCollection::at(size_t index) const
{
    Susie* susie;

    try
    {
        susie = &pimpl->plugins.at(index);
    }
    catch ( std::out_of_range& )
    {
        susie = nullptr;
    }

    return susie;
}

//---------------------------------------------------------------------------//
//
// Members
//
//---------------------------------------------------------------------------//

HRESULT __stdcall SusieCollection::Append(LPCTSTR path)
{
    console_out(TEXT("SusieCollection::Append() begin"));

    Susie susie(path);

    pimpl->cs.lock();
    {
        pimpl->plugins.emplace_back(std::move(susie));
    }
    pimpl->cs.unlock();

    return S_OK;

    console_out(TEXT("SusieCollection::Append() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SusieCollection::Remove(LPCTSTR path)
{
    console_out(TEXT("SusieCollection::Remove() begin"));

    HRESULT hr = S_FALSE;

    pimpl->cs.lock();
    {
        const auto end = pimpl->plugins.end();
        for ( auto it = pimpl->plugins.begin(); it != end; ++it )
        {
            const auto& plugin = *it;
            if ( 0 == lstrcmp(path, plugin.path()) )
            {
                pimpl->plugins.erase(it);
                console_out(TEXT("removed"));

                hr = S_OK;
                break;
            }
        }
    }
    pimpl->cs.unlock();

    console_out(TEXT("SusieCollection::Remove() end"));

    return hr;
}

//---------------------------------------------------------------------------//

SusieCollectionBase* __stdcall SusieCollection::Collect
(
    Condition condition
)
{
    SusieCollection* collection = nullptr;

    pimpl->cs.lock();
    {
        for ( auto& plugin : pimpl->plugins )
        {
            if ( condition )
            {
                collection->Append(plugin.path());
            }
        }
    }
    pimpl->cs.unlock();

    return collection;
}

//---------------------------------------------------------------------------//

Susie* __stdcall SusieCollection::Find
(
    LPCTSTR path
)
{
    Susie* susie = nullptr;

    pimpl->cs.lock();
    {
        for ( auto& plugin : pimpl->plugins )
        {
            if ( 0 == lstrcmp(path, plugin.path()) )
            {
                susie = &plugin;
                break;
            }
        }
    }
    pimpl->cs.unlock();

    return susie;
}

//---------------------------------------------------------------------------//

// SusieCollection.cpp