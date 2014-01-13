// ComponentBase.cpp

//---------------------------------------------------------------------------//
//
// コンポーネントの基底クラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <queue>
#include <string>
#include <unordered_map>

#include <windows.h>

#include "ComPtr.hpp"
#include "DebugPrint.hpp"
#include "UString.hpp"

#include "ComponentBase.hpp"

//---------------------------------------------------------------------------//

using namespace  CubeMelon;

//---------------------------------------------------------------------------//
//
// コンポーネントの基底クラス
//
//---------------------------------------------------------------------------//

Component::Component(IUnknown* pUnkOuter)
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

    m_msg_map = new MessageMap;

    m_state = STATE::IDLE;

    console_out(TEXT("%s::Base::ctor() end"), NAME);
}

//---------------------------------------------------------------------------//

Component::~Component()
{
    console_out(TEXT("%s::Base::dtor() begin"), NAME);

    if ( m_property )
    {
        m_property->Release();
        m_property = nullptr;
    }
    if ( m_owner )
    {
        m_owner->Release();
        m_owner = nullptr;
    }

    delete m_msg_map;
    m_msg_map = nullptr;

    m_state = STATE::IDLE;

    console_out(TEXT("%s::Base::dtor() end"), NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    console_out(TEXT("%s::Base::QueryInterface() begin"), NAME);

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
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

//---------------------------------------------------------------------------//

ULONG __stdcall Component::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::Base::AddRef(): %d -> %d"), NAME, cRef - 1, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Component::Release()
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

//---------------------------------------------------------------------------//

REFCLSID __stdcall Component::clsid() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

ICompCollection* __stdcall Component::collection() const
{
    return m_owner ? m_owner->collection() : nullptr;
}

//---------------------------------------------------------------------------//

U8CSTR __stdcall Component::name() const
{
    return COMP_NAME;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall Component::owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IDataArray* __stdcall Component::property() const
{
    return m_property;
}

//---------------------------------------------------------------------------//

STATE __stdcall Component::status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::AttachMessage
(
    U8CSTR msg, IComponent* listener
)
{
    console_out(TEXT("%s::Base::AttachMessage() begin"), NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    console_out(TEXT("msg: %s"),      toUnicodez(msg));
    console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

    const auto range = m_msg_map->equal_range(std::move(std::u8string(msg)));
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

    m_msg_map->emplace(msg, listener);

    console_out(TEXT("%s::Base::AttachMessage() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::DetachMessage
(
    U8CSTR msg, IComponent* listener
)
{
    console_out(TEXT("%s::Base::DetachMessage() begin"), NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    console_out(TEXT("msg: %s"),      toUnicodez(msg));
    console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

    size_t count = 0;
    const auto range = m_msg_map->equal_range(std::move(std::u8string(msg)));
    const auto end = range.second;
    auto it = range.first;
     while ( it != end )
    {
        if ( it->second == listener )
        {
            m_msg_map->erase(it);
            ++count;
            break;
        }
        ++it;
    }
    if ( count == 0 )
    {
        console_out(TEXT("This message was not attached to the component"));
        console_out(TEXT("%s::Base::DetachMessage() end"), NAME);
        return S_FALSE;
    }

    console_out(TEXT("%s::Base::DetachMessage() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::NotifyMessage
(
    U8CSTR msg, IComponent* sender, IData* data
)
{
    console_out(TEXT("%s::Base::NotifyMessage() begin"), NAME);

    // デバッグ情報の表示
    console_outA("msg:    %s", (const char*)msg);
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

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::OpenConfiguration
(
    HWND hwndParent, HWND* phwnd
)
{
    console_out(TEXT("%s:Base::OpenConfiguration(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::Start
(
    void* args, IComponent* listener
)
{
    console_out(TEXT("%s:Base::Start(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Component::Stop
(
    void* args, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Stop(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//
//
// 入力コンポーネントの基底クラス
//
//---------------------------------------------------------------------------//

ReaderComponent::ReaderComponent(IUnknown* pUnkOuter)
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

    m_msg_map = new MessageMap;

    m_state = STATE::IDLE;

    console_out(TEXT("%s::Base::ctor() end"), NAME);
}

//---------------------------------------------------------------------------//

ReaderComponent::~ReaderComponent()
{
    console_out(TEXT("%s::Base::dtor() begin"), NAME);

    if ( m_property )
    {
        m_property->Release();
        m_property = nullptr;
    }
    if ( m_owner )
    {
        m_owner->Release();
        m_owner = nullptr;
    }

    delete m_msg_map;
    m_msg_map = nullptr;

    m_state = STATE::IDLE;

    console_out(TEXT("%s::Base::dtor() end"), NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    console_out(TEXT("%s::Base::QueryInterface() begin"), NAME);

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
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

//---------------------------------------------------------------------------//

ULONG __stdcall ReaderComponent::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::Base::AddRef(): %d -> %d"), NAME, cRef - 1, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall ReaderComponent::Release()
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

//---------------------------------------------------------------------------//

REFCLSID __stdcall ReaderComponent::clsid() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

ICompCollection* __stdcall ReaderComponent::collection() const
{
    return m_owner ? m_owner->collection() : nullptr;
}

//---------------------------------------------------------------------------//

U8CSTR __stdcall ReaderComponent::name() const
{
    return COMP_NAME;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall ReaderComponent::owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IDataArray* __stdcall ReaderComponent::property() const
{
    return m_property;
}

//---------------------------------------------------------------------------//

STATE __stdcall ReaderComponent::status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::AttachMessage
(
    U8CSTR msg, IComponent* listener
)
{
    console_out(TEXT("%s::Base::AttachMessage() begin"), NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    console_out(TEXT("msg: %s"),      toUnicodez(msg));
    console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

    const auto range = m_msg_map->equal_range(std::move(std::u8string(msg)));
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

    m_msg_map->emplace(msg, listener);

    console_out(TEXT("%s::Base::AttachMessage() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::DetachMessage
(
    U8CSTR msg, IComponent* listener
)
{
    console_out(TEXT("%s::Base::DetachMessage() begin"), NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    console_out(TEXT("msg: %s"),      toUnicodez(msg));
    console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

    size_t count = 0;
    const auto range = m_msg_map->equal_range(std::move(std::u8string(msg)));
    const auto end = range.second;
    auto it = range.first;
     while ( it != end )
    {
        if ( it->second == listener )
        {
            m_msg_map->erase(it);
            ++count;
            break;
        }
        ++it;
    }
    if ( count == 0 )
    {
        console_out(TEXT("This message was not attached to the component"));
        console_out(TEXT("%s::Base::DetachMessage() end"), NAME);
        return S_FALSE;
    }

    console_out(TEXT("%s::Base::DetachMessage() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::NotifyMessage
(
    U8CSTR msg, IComponent* sender, IData* data
)
{
    console_out(TEXT("%s::Base::NotifyMessage() begin"), NAME);

    // デバッグ情報の表示
    console_outA("msg:    %s", (const char*)msg);
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

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::OpenConfiguration
(
    HWND hwndParent, HWND* phwnd
)
{
    console_out(TEXT("%s:Base::OpenConfiguration(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::Start
(
    void* args, IComponent* listener
)
{
    console_out(TEXT("%s:Base::Start(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::Stop
(
    void* args, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Stop(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::Close
(
    IComponent* listener
)
{
    console_out(TEXT("%s::Base::Close(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::Open
(
    U8CSTR path, U8CSTR format_as, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Open(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::QuerySupport
(
    U8CSTR path, U8CSTR format_as
)
{
    console_out(TEXT("%s::Base::QuerySupport(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ReaderComponent::Read
(
    void* buffer, size_t offset, size_t buf_size, size_t* cb_data, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Read(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//
//
// 出力コンポーネントの基底クラス
//
//---------------------------------------------------------------------------//

WriterComponent::WriterComponent(IUnknown* pUnkOuter)
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

    m_msg_map = new MessageMap;

    m_state = STATE::IDLE;

    console_out(TEXT("%s::Base::ctor() end"), NAME);
}

//---------------------------------------------------------------------------//

WriterComponent::~WriterComponent()
{
    console_out(TEXT("%s::Base::dtor() begin"), NAME);

    if ( m_property )
    {
        m_property->Release();
        m_property = nullptr;
    }
    if ( m_owner )
    {
        m_owner->Release();
        m_owner = nullptr;
    }

    delete m_msg_map;
    m_msg_map = nullptr;

    m_state = STATE::IDLE;

    console_out(TEXT("%s::Base::dtor() end"), NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    console_out(TEXT("%s::Base::QueryInterface() begin"), NAME);

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
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

//---------------------------------------------------------------------------//

ULONG __stdcall WriterComponent::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::Base::AddRef(): %d -> %d"), NAME, cRef - 1, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall WriterComponent::Release()
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

//---------------------------------------------------------------------------//

REFCLSID __stdcall WriterComponent::clsid() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

ICompCollection* __stdcall WriterComponent::collection() const
{
    return m_owner ? m_owner->collection() : nullptr;
}

//---------------------------------------------------------------------------//

U8CSTR __stdcall WriterComponent::name() const
{
    return COMP_NAME;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall WriterComponent::owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IDataArray* __stdcall WriterComponent::property() const
{
    return m_property;
}

//---------------------------------------------------------------------------//

STATE __stdcall WriterComponent::status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::AttachMessage
(
    U8CSTR msg, IComponent* listener
)
{
    console_out(TEXT("%s::Base::AttachMessage() begin"), NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    console_out(TEXT("msg: %s"),      toUnicodez(msg));
    console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

    const auto range = m_msg_map->equal_range(std::move(std::u8string(msg)));
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

    m_msg_map->emplace(msg, listener);

    console_out(TEXT("%s::Base::AttachMessage() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::DetachMessage
(
    U8CSTR msg, IComponent* listener
)
{
    console_out(TEXT("%s::Base::DetachMessage() begin"), NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    console_out(TEXT("msg: %s"),      toUnicodez(msg));
    console_out(TEXT("listener: %s"), toUnicodez(listener->name()));

    size_t count = 0;
    const auto range = m_msg_map->equal_range(std::move(std::u8string(msg)));
    const auto end = range.second;
    auto it = range.first;
     while ( it != end )
    {
        if ( it->second == listener )
        {
            m_msg_map->erase(it);
            ++count;
            break;
        }
        ++it;
    }
    if ( count == 0 )
    {
        console_out(TEXT("This message was not attached to the component"));
        console_out(TEXT("%s::Base::DetachMessage() end"), NAME);
        return S_FALSE;
    }

    console_out(TEXT("%s::Base::DetachMessage() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::NotifyMessage
(
    U8CSTR msg, IComponent* sender, IData* data
)
{
    console_out(TEXT("%s::Base::NotifyMessage() begin"), NAME);

    // デバッグ情報の表示
    console_outA("msg:    %s", (const char*)msg);
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

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::OpenConfiguration
(
    HWND hwndParent, HWND* phwnd
)
{
    console_out(TEXT("%s:Base::OpenConfiguration(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::Start
(
    void* args, IComponent* listener
)
{
    console_out(TEXT("%s:Base::Start(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::Stop
(
    void* args, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Stop(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::QuerySupport
(
    U8CSTR path, U8CSTR format_as
)
{
    console_out(TEXT("%s::Base::QuerySupport(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::Open
(
    U8CSTR path, U8CSTR format_as, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Open(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::Close
(
    IComponent* listener
)
{
    console_out(TEXT("%s::Base::Close(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WriterComponent::Write
(
    void* buffer, size_t offset, size_t buf_size, size_t* cb_data, IComponent* listener
)
{
    console_out(TEXT("%s::Base::Write(): Not Implemented"), NAME);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

// ComponentBase.cpp