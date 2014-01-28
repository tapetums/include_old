// Data.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#include "DebugPrint.hpp"
#include "GenerateUUIDString.hpp"
#include "Interfaces.hpp"

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

static const size_t MAX_DATA_NAME_LENGTH = 64;

//---------------------------------------------------------------------------//

class CData : public IData
{
public:
    explicit CData(U8CSTR name = nullptr)
    {
        m_cRef = 1;

        if ( name )
        {
            ::StringCchCopyA((char*)m_name, MAX_DATA_NAME_LENGTH, (const char*)name);
        }
        else
        {
            ::GenerateUUIDStringA((char*)m_name, MAX_DATA_NAME_LENGTH);
        }
    }

    virtual ~CData()
    {
    }

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
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

        this->AddRef();

        return S_OK;
    }

    ULONG __stdcall AddRef() override
    {
        return ::InterlockedIncrement(&m_cRef);
    }

    ULONG __stdcall Release() override
    {
        const auto cRef = ::InterlockedDecrement(&m_cRef);
        if ( cRef == 0 )
        {
            delete this;
        }
        return cRef;
    }

public:
    U8CSTR __stdcall name() const override { return m_name; }

protected:
    ULONG   m_cRef;
    char8_t m_name[MAX_DATA_NAME_LENGTH];

private:
    CData(const CData&)             = delete;
    CData(CData&&)                  = delete;
    CData& operator =(const CData&) = delete;
    CData& operator =(CData&&)      = delete;
};

//---------------------------------------------------------------------------//

template <typename T> class Data : public CData
{
public:
    explicit Data(U8CSTR name, const T& data) : CData(name)
    {
        m_data = data;
        console_outA("Data<T>::ctor(%s)", (char*)m_name);
    }

    ~Data()
    {
        console_outA("Data<T>::dtor(%s)", (char*)m_name);
    }

public:
    uint64_t  __stdcall size() const override { return sizeof(T); }
    uintptr_t __stdcall get()  const override { return (uintptr_t)m_data; }
    HRESULT   __stdcall set(uintptr_t value) override { m_data = (T)value; return S_OK; }

protected:
    T m_data;
};

//---------------------------------------------------------------------------//

template <> class Data<void> : public CData
{
public:
    explicit Data(U8CSTR name = nullptr) : CData(name)
    {
        console_outA("Data<void>::ctor(%s)", (char*)m_name);
    }

    ~Data()
    {
        console_outA("Data<void>::dtor(%s)", (char*)m_name);
    }

public:
    uint64_t  __stdcall size() const override { return 0; }
    uintptr_t __stdcall get()  const override { return 0; }
    HRESULT   __stdcall set(uintptr_t value) override { return S_FALSE; }
};

//---------------------------------------------------------------------------//

template <> class Data<void*> : public CData
{
public:
    explicit Data(U8CSTR name, void* data, uint64_t size) : CData(name)
    {
        m_data = data;
        m_size = size;
        console_outA("Data<void*>::ctor(%s)", (char*)m_name);
    }

    ~Data()
    {
        console_outA("Data<void*>::dtor(%s)", (char*)m_name);
    }

public:
    uint64_t  __stdcall size() const override { return m_size; }
    uintptr_t __stdcall get()  const override { return (uintptr_t)m_data; }
    HRESULT   __stdcall set(uintptr_t value) override { m_data = (void*)value; return S_OK; }

protected:
    void*    m_data;
    uint64_t m_size;
};

//---------------------------------------------------------------------------//

template <class T> class Data<T*> : public CData
{
public:
    explicit Data(U8CSTR name, const T* data = nullptr) : CData(name)
    {
        if ( data )
        {
            m_data = *data;
        }
        else
        {
            m_data = T();
        }
        console_outA("Data<T*>::ctor(%s)", (char*)m_name);
    }

    explicit Data(U8CSTR name, const T& data) : CData(name)
    {
        m_data = data;
        console_outA("Data<T*>::ctor(%s)", (char*)m_name);
    }

    ~Data()
    {
        console_outA("Data<T*>::dtor(%s)", (char*)m_name);
    }

public:
    uint64_t  __stdcall size() const override { return sizeof(T); }
    uintptr_t __stdcall get()  const override { return (uintptr_t)&m_data; }
    HRESULT   __stdcall set(uintptr_t value) override
    {
        const auto p_data = (T*)value;
        if ( nullptr == p_data )
        {
            return E_INVALIDARG;
        }
        else
        {
            m_data = *p_data;
            return S_OK;
        }
    }

protected:
    T m_data;
};

//---------------------------------------------------------------------------//

class CDataArray : public IDataArray
{
public:
    CDataArray()
    {
    }

    virtual ~CDataArray()
    {
    }

public:
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
        else if ( IsEqualIID(riid, IID_IDataArray) )
        {
            *ppvObject = static_cast<IDataArray*>(this);
        }
        else
        {
            return E_NOINTERFACE;
        }

        this->AddRef();

        return S_OK;
    }

    ULONG __stdcall AddRef() override
    {
        return ::InterlockedIncrement(&m_cRef);
    }

    ULONG __stdcall Release() override
    {
        auto cRef = ::InterlockedDecrement(&m_cRef);
        if ( cRef == 0 )
        {
            delete this;
        }
        return cRef;
    }

protected:
    ULONG m_cRef = 1;

private:
    CDataArray(const CDataArray&)             = delete;
    CDataArray(CDataArray&&)                  = delete;
    CDataArray& operator =(const CDataArray&) = delete;
    CDataArray& operator =(CDataArray&&)      = delete;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Data.hpp