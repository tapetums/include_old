// Property.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <Interfaces.hpp>

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component;

extern const size_t   COMP_INDEX;
extern const char8_t* COPYRIGHT;
extern const char8_t* COMP_DESC;
extern const char8_t* COMP_NAME;

extern const CubeMelon::COMPTYPE COMP_TYPE;
extern const CubeMelon::VerInfo  VERINFO;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class Property : public IDataArray
{
public:
    Property();
    ~Property();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

public:
    IData* __stdcall at(size_t index = 0) const override;
    size_t __stdcall size()               const override;

private:
    struct Impl;
    Impl* pimpl;

private:
    Property(const Property&)             = delete;
    Property(Property&&)                  = delete;
    Property& operator =(const Property&) = delete;
    Property& operator =(Property&&)      = delete;
};

//---------------------------------------------------------------------------//

} //namespace CubeMelon

//---------------------------------------------------------------------------//

extern CubeMelon::Property g_property;

//---------------------------------------------------------------------------//

// Property.hpp