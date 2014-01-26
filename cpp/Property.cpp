// Property.cpp

#include <windows.h>

#include "Interfaces.hpp"

#include "Property.hpp"

//---------------------------------------------------------------------------//

CubeMelon::Property g_property;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

enum class PROPERTY : size_t
{
    API_VER,
    CLSID,
    COPYRIGHT,
    DESCRIPTION,
    NAME,
    TYPE,
    VERSION,
    COUNT
};

//---------------------------------------------------------------------------//

struct PropData : public IData
{
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
        if ( nullptr == ppvObject )
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;
        if ( IsEqualIID(riid, IID_IUnknown) )
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if ( IsEqualIID(riid, IID_IData) )
        {
            *ppvObject = static_cast<IData*>(this);
        }
        else
        {
            return E_NOINTERFACE;
        }

        return S_OK;
    }

    ULONG __stdcall AddRef()
    {
        return 2;
    }

    ULONG __stdcall Release()
    {
        return 1;
    }

    HRESULT __stdcall set(uintptr_t value) override
    {
        return E_NOTIMPL;
    }
};

//---------------------------------------------------------------------------//

struct ApiVer : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"apiver";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(VerInfo*);
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)(VerInfo*)&API_VERSION;
    }
};

//---------------------------------------------------------------------------//

struct Clsid : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"clsid";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(CLSID*);
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)(CLSID*)&CLSID_Component;
    }
};

//---------------------------------------------------------------------------//

struct Copyright : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"copyright";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(char8_t) * (1 + strlen((const char*)COPYRIGHT));
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)COPYRIGHT;
    }
};

//---------------------------------------------------------------------------//

struct Description : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"description";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(char8_t) * (1 + strlen((const char*)COMP_DESC));
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)COMP_DESC;
    }
};

//---------------------------------------------------------------------------//

struct Name : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"name";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(char8_t) * (1 + strlen((const char*)COMP_NAME));
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)COMP_NAME;
    }
};

//---------------------------------------------------------------------------//

struct Type : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"type";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(COMPTYPE);
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)COMP_TYPE;
    }
};

//---------------------------------------------------------------------------//

struct Version : public PropData
{
    U8CSTR __stdcall name() const override
    {
        return (U8CSTR)"version";
    }

    uint64_t __stdcall size() const override
    {
        return sizeof(VerInfo*);
    }

    uintptr_t __stdcall get() const override
    {
        return (uintptr_t)(VerInfo*)&VERINFO;
    }
};

//---------------------------------------------------------------------------//

struct Property::Impl
{
    ApiVer      apiver;
    Clsid       clsid;
    Copyright   copyright;
    Description description;
    Name        name;
    Type        type;
    Version     version;
};

//---------------------------------------------------------------------------//

Property::Property()
{
    pimpl = new Impl;
}

//---------------------------------------------------------------------------//

Property::~Property()
{
    delete pimpl;
    pimpl = nullptr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Property::QueryInterface(REFIID riid, void** ppvObject)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;
    if ( IsEqualIID(riid, IID_IUnknown) )
    {
        *ppvObject = static_cast<IUnknown*>(this);
    }
    else if ( IsEqualIID(riid, IID_IDataArray) )
    {
        *ppvObject = static_cast<IDataArray*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall Property::AddRef()
{
    return 2;
}

//---------------------------------------------------------------------------//

ULONG __stdcall Property::Release()
{
    return 1;
}

//---------------------------------------------------------------------------//

IData* __stdcall Property::at(size_t index) const
{
    switch ( index )
    {
        case (size_t)PROPERTY::API_VER:     return &pimpl->apiver;
        case (size_t)PROPERTY::CLSID:       return &pimpl->clsid;
        case (size_t)PROPERTY::COPYRIGHT:   return &pimpl->copyright;
        case (size_t)PROPERTY::DESCRIPTION: return &pimpl->description;
        case (size_t)PROPERTY::NAME:        return &pimpl->name;
        case (size_t)PROPERTY::TYPE:        return &pimpl->type;
        case (size_t)PROPERTY::VERSION:     return &pimpl->version;
        default:                            return nullptr;
    }
}

//---------------------------------------------------------------------------//

U8CSTR __stdcall Property::name() const
{
    return COMP_NAME;
}

//---------------------------------------------------------------------------//

size_t __stdcall Property::size() const
{
    return (size_t)PROPERTY::COUNT;
}

//---------------------------------------------------------------------------//

} //namespace CubeMelon

//---------------------------------------------------------------------------//

// Property.cpp