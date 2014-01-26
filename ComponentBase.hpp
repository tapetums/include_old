// ComponentBase.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// コンポーネントの基底クラス (Mix-in)
//   Copyright (C) 2013-2014 tapetums
//
//---------------------------------------------------------------------------//

#include <string>
#include <unordered_map>

#include <windows.h>

#include "ComPtr.hpp"
#include "DebugPrint.hpp"
#include "UString.hpp"
#include "Interfaces.hpp"

//---------------------------------------------------------------------------//
//
// 前方宣言
//
//---------------------------------------------------------------------------//

namespace std { typedef basic_string<char8_t> u8string; }

typedef std::unordered_multimap<std::u8string, ComPtr<CubeMelon::IComponent>> MessageMap;

//---------------------------------------------------------------------------//

extern const TCHAR*   NAME;
extern const CLSID    CLSID_Component;
extern const char8_t* COMP_NAME;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//
//
// Mix-in クラス
//
//---------------------------------------------------------------------------//

template <class I> class ComponentBase : public I
{
public:
    explicit ComponentBase(IUnknown* pUnkOuter = nullptr)
    {
        console_out(TEXT("%s::Base::ctor() begin"), NAME);

        if ( pUnkOuter )
        {
            console_out(TEXT("%s::Base::GetOwnerInterface begin"), NAME);
            pUnkOuter->QueryInterface
            (
                IID_IComponent, (void**)&m_owner
            );
            console_out(TEXT("%s::Base::GetOwnerInterface end"), NAME);
        }

        this->AddRef();

        m_state = STATE::IDLE;

        console_out(TEXT("%s::Base::ctor() end"), NAME);
    }

    virtual ~ComponentBase()
    {
        console_out(TEXT("%s::Base::dtor() begin"), NAME);

        m_state = STATE::UNKNOWN;

        console_out(TEXT("%s::Base::dtor() end"), NAME);
    }

public:
    ULONG __stdcall AddRef()
    {
        LONG cRef = ::InterlockedIncrement(&m_cRef);

        console_out(TEXT("%s::Base::AddRef(): %d -> %d"), NAME, cRef - 1, cRef);

        return static_cast<ULONG>(cRef);
    }

    ULONG __stdcall Release()
    {
        if ( m_cRef < 1 )
        {
            console_out(TEXT("%s::Base::Release() %d"), NAME, m_cRef);
            return m_cRef;
        }

        LONG cRef = ::InterlockedDecrement(&m_cRef);

        console_out(TEXT("%s::Base::Release(): %d -> %d"), NAME, cRef + 1, cRef);

        if ( cRef == 0 )
        {
            console_out(TEXT("%s::Base::delete begin"), NAME);
            {
                delete this;
            }
            console_out(TEXT("%s::Base::delete end"), NAME);
        }

        return static_cast<ULONG>(cRef);
    }

public:
    REFCLSID __stdcall clsid() const
    {
        return CLSID_Component;
    }

    ICompCollection* __stdcall collection() const
    {
        return m_owner ? m_owner->collection() : nullptr;
    }

    U8CSTR __stdcall name() const
    {
        return COMP_NAME;
    }

    IComponent* __stdcall owner() const
    {
        return m_owner.GetInterface();
    }

    IDataArray* __stdcall property() const
    {
        return m_property.GetInterface();
    }

    STATE __stdcall status() const
    {
        return m_state;
    }

public:
    HRESULT __stdcall AttachMessage
    (
        U8CSTR msg, IComponent* listener
    )
    {
        console_out(TEXT("%s::Base::AttachMessage() begin"), NAME);

        if ( nullptr == listener )
        {
            return E_INVALIDARG;
        }

        console_out(TEXT("msg: %s"),      toUnicodez(msg));
        console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

        const auto range = m_msg_map.equal_range(std::move(std::u8string(msg)));
        const auto end = range.second;
        auto it = range.first;
        while ( it != end )
        {
            if ( it->second == listener )
            {
                console_out(TEXT("This message has been already attached to the component"));
                console_out(TEXT("%s::Base::AttachMessage() end"), NAME);
                return S_FALSE;
            }
            ++it;
        }

        m_msg_map.emplace(msg, listener);

        console_out(TEXT("%s::Base::AttachMessage() end"), NAME);

        return S_OK;
    }

    HRESULT __stdcall DetachMessage
    (
        U8CSTR msg, IComponent* listener
    )
    {
        console_out(TEXT("%s::Base::DetachMessage() begin"), NAME);

        if ( nullptr == listener )
        {
            return E_INVALIDARG;
        }

        console_out(TEXT("msg: %s"),      toUnicodez(msg));
        console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

        const auto range = m_msg_map.equal_range(std::move(std::u8string(msg)));
        const auto end = range.second;
        auto it = range.first;
        while ( it != end )
        {
            if ( it->second == listener )
            {
                m_msg_map.erase(it);
                break;
            }
            ++it;
        }
        if ( it == end )
        {
            console_out(TEXT("This message was not attached to the component"));
            console_out(TEXT("%s::Base::DetachMessage() end"), NAME);
            return S_FALSE;
        }

        console_out(TEXT("%s::Base::DetachMessage() end"), NAME);

        return S_OK;
    }

    HRESULT __stdcall NotifyMessage
    (
        U8CSTR msg, IComponent* sender = nullptr, IData* data = nullptr
    )
    {
        console_out(TEXT("%s::Base::NotifyMessage() begin"), NAME);

        // デバッグ情報の表示
        console_outA("msg:    %s", toMBCSz(msg));
        console_outA("sender: %s", sender ? toMBCSz(sender->name()) : "(null)");
        console_outA("name:   %s",       data ? toMBCSz(data->name()) : "(null)");
        console_outA("data:   0x%08x",   data ? data->get()  : 0);
        console_outA("size:   %u bytes", data ? data->size() : 0);

        // データの解放
        if ( data )
        {
            data->Release();
            data = nullptr;
        }

        console_out(TEXT("%s::Base::NotifyMessage() end"), NAME);

        return S_OK;
    }

    HRESULT __stdcall OpenConfiguration
    (
        HWND hwndParent, HWND* phwnd
    )
    {
        console_out(TEXT("%s:Base::OpenConfiguration() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Start
    (
        void* args, IComponent* listener = nullptr
    )
    {
        console_out(TEXT("%s:Base::Start() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Stop
    (
        void* args, IComponent* listener = nullptr
    )
    {
        console_out(TEXT("%s::Base::Stop() not implemented"), NAME);
        return E_NOTIMPL;
    }

protected:
    ULONG m_cRef  = 0;
    STATE m_state = STATE::UNKNOWN;

    ComPtr<IComponent> m_owner;
    ComPtr<IDataArray> m_property;
    MessageMap         m_msg_map;

private:
    ComponentBase(const ComponentBase&)             = delete;
    ComponentBase(ComponentBase&&)                  = delete;
    ComponentBase& operator =(const ComponentBase&) = delete;
    ComponentBase& operator =(ComponentBase&&)      = delete;
};

//---------------------------------------------------------------------------//

template <class I> class IOComponentBase : public ComponentBase<I>
{
public:
    explicit IOComponentBase(IUnknown* pUnkOuter) : ComponentBase(pUnkOuter)
    {
    }

public:
    HRESULT __stdcall QuerySupport
    (
        U8CSTR path, U8CSTR format_as
    )
    {
        console_out(TEXT("%s::Base::QuerySupport() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Open
    (
        U8CSTR path, U8CSTR format_as, IComponent* listener = nullptr
    )
    {
        console_out(TEXT("%s::Base::Open() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Close
    (
        IComponent* listener = nullptr
    )
    {
        console_out(TEXT("%s::Base::Close() not implemented"), NAME);
        return E_NOTIMPL;
    }
};

//---------------------------------------------------------------------------//
//
// コンポーネントの基底クラス
//
//---------------------------------------------------------------------------//

class Component : public ComponentBase<IComponent>
{
public:
    explicit Component(IUnknown* pUnkOuter) : ComponentBase(pUnkOuter)
    {
    }

public:
    HRESULT __stdcall QueryInterface
    (
        REFIID riid, void** ppvObject
    )
    {
        console_out(TEXT("%s::Base::QueryInterface() begin"), NAME);

        if ( nullptr == ppvObject )
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;

        console_out
        (
            TEXT("IID: { %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X }"),
            riid.Data1, riid.Data2, riid.Data3,
            riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
            riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]
        );
        if ( IsEqualIID(riid, IID_IUnknown) )
        {
            console_out(TEXT("IID_IUnknown"));
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if ( IsEqualIID(riid, IID_IComponent) )
        {
            console_out(TEXT("IID_IComponent"));
            *ppvObject = static_cast<IComponent*>(this);
        }
        else
        {
            console_out(TEXT("No such an interface"));
            console_out(TEXT("%s::Base::QueryInterface() end"), NAME);
            return E_NOINTERFACE;
        }

        this->AddRef();

        console_out(TEXT("%s::Base::QueryInterface() end"), NAME);

        return S_OK;
    }
};

//---------------------------------------------------------------------------//

class ReaderComponent : public IOComponentBase<IReaderComponent>
{
public:
    explicit ReaderComponent(IUnknown* pUnkOuter) : IOComponentBase(pUnkOuter)
    {
    }

public:
    HRESULT __stdcall QueryInterface
    (
        REFIID riid, void** ppvObject
    )
    {
        console_out(TEXT("%s::Base::QueryInterface() begin"), NAME);

        if ( nullptr == ppvObject )
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;

        console_out
        (
            TEXT("IID: { %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X }"),
            riid.Data1, riid.Data2, riid.Data3,
            riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
            riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]
        );
        if ( IsEqualIID(riid, IID_IUnknown) )
        {
            console_out(TEXT("IID_IUnknown"));
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if ( IsEqualIID(riid, IID_IComponent) )
        {
            console_out(TEXT("IID_IComponent"));
            *ppvObject = static_cast<IComponent*>(this);
        }
        else if ( IsEqualIID(riid, IID_IIOComponent) )
        {
            console_out(TEXT("IID_IIOComponent"));
            *ppvObject = static_cast<IIOComponent*>(this);
        }
        else if ( IsEqualIID(riid, IID_IReaderComponent) )
        {
            console_out(TEXT("IID_IReaderComponent"));
            *ppvObject = static_cast<IReaderComponent*>(this);
        }
        else
        {
            console_out(TEXT("No such an interface"));
            console_out(TEXT("%s::Base::QueryInterface() end"), NAME);
            return E_NOINTERFACE;
        }

        this->AddRef();

        console_out(TEXT("%s::Base::QueryInterface() end"), NAME);

        return S_OK;
    }

public:
    HRESULT __stdcall Read
    (
        void* buffer, size_t offset, size_t buf_size, size_t* cb_data,
        IComponent* listener = nullptr
    )
    {
        console_out(TEXT("%s::Base::Read() not implemented"), NAME);
        return E_NOTIMPL;
    }
};

//---------------------------------------------------------------------------//

class WriterComponent : public IOComponentBase<IWriterComponent>
{
public:
    explicit WriterComponent(IUnknown* pUnkOuter) : IOComponentBase(pUnkOuter)
    {
    }

public:
    HRESULT __stdcall QueryInterface
    (
        REFIID riid, void** ppvObject
    )
    {
        console_out(TEXT("%s::Base::QueryInterface() begin"), NAME);

        if ( nullptr == ppvObject )
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;

        console_out
        (
            TEXT("IID: { %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X }"),
            riid.Data1, riid.Data2, riid.Data3,
            riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
            riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]
        );
        if ( IsEqualIID(riid, IID_IUnknown) )
        {
            console_out(TEXT("IID_IUnknown"));
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if ( IsEqualIID(riid, IID_IComponent) )
        {
            console_out(TEXT("IID_IComponent"));
            *ppvObject = static_cast<IComponent*>(this);
        }
        else if ( IsEqualIID(riid, IID_IIOComponent) )
        {
            console_out(TEXT("IID_IIOComponent"));
            *ppvObject = static_cast<IIOComponent*>(this);
        }
        else if ( IsEqualIID(riid, IID_IWriterComponent) )
        {
            console_out(TEXT("IID_IWriterComponent"));
            *ppvObject = static_cast<IWriterComponent*>(this);
        }
        else
        {
            console_out(TEXT("No such an interface"));
            console_out(TEXT("%s::Base::QueryInterface() end"), NAME);
            return E_NOINTERFACE;
        }

        this->AddRef();

        console_out(TEXT("%s::Base::QueryInterface() end"), NAME);

        return S_OK;
    }

public:
    HRESULT __stdcall Write
    (
        void* buffer, size_t offset, size_t buf_size, size_t* cb_data,
        IComponent* listener = nullptr
    )
    {
        console_out(TEXT("%s::Base::Write() not implemented"), NAME);
        return E_NOTIMPL;
    }
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// ComponentBase.hpp