﻿// ComponentBase.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// コンポーネントの基底クラス (Mix-in)
//   Copyright (C) 2013-2014 tapetums
//
//---------------------------------------------------------------------------//

#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#include <windows.h>

#include "ComPtr.hpp"
#include "DebugPrint.hpp"
#include "Interfaces.hpp"
#include "CriticalSection.hpp"
#include "Data.hpp"

//---------------------------------------------------------------------------//
//
// 前方宣言
//
//---------------------------------------------------------------------------//

namespace std { typedef basic_string<char8_t> u8string; }

typedef ComPtr<CubeMelon::IComponent>                        ComponentPtr;
typedef ComPtr<CubeMelon::IDataArray>                        DataArrayPtr;
typedef std::unordered_multimap<std::u8string, ComponentPtr> MessageMap;
typedef std::queue<CubeMelon::NotifyData>                    NotifyQueue;

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
protected:
    ULONG m_cRef  = 0;
    STATE m_state = STATE::UNKNOWN;
    bool  m_alive = true;

    ComponentPtr    m_owner;
    DataArrayPtr    m_property;
    MessageMap      m_msg_map;

    CriticalSection m_cs_notify;
    HANDLE          m_evt_notify;
    NotifyQueue     m_q_notify;
    std::thread     m_thread_notify;

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

        m_thread_notify = std::thread([=]()
        {
            // COMの初期化
            ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

            // メインループ
            m_evt_notify = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

            // メインループ
            this->NotifyMessageLoop();

            // イベントの破棄
            if ( m_evt_notify )
            {
                ::CloseHandle(m_evt_notify);
                m_evt_notify = nullptr;
            }

            // COMの後処理
            ::CoUninitialize();
        });

        m_state = STATE::IDLE;

        console_out(TEXT("%s::Base::ctor() end"), NAME);
    }

    virtual ~ComponentBase()
    {
        console_out(TEXT("%s::Base::dtor() begin"), NAME);

        if ( m_thread_notify.joinable() )
        {
            m_alive = false;
            ::SetEvent(m_evt_notify);
            m_thread_notify.join();
        }

        m_state = STATE::UNKNOWN;

        console_out(TEXT("%s::Base::dtor() end"), NAME);
    }

public:
    ULONG __stdcall AddRef() override
    {
        const LONG cRef = ::InterlockedIncrement(&m_cRef);

        console_out(TEXT("%s::Base::AddRef(): %d -> %d"), NAME, cRef - 1, cRef);

        return static_cast<ULONG>(cRef);
    }

    ULONG __stdcall Release() override
    {
        const LONG cRef = ::InterlockedDecrement(&m_cRef);

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
    REFCLSID __stdcall clsid() const override
    {
        return CLSID_Component;
    }

    ICompCollection* __stdcall collection() const override
    {
        return m_owner ? m_owner->collection() : nullptr;
    }

    U8CSTR __stdcall name() const override
    {
        return COMP_NAME;
    }

    IComponent* __stdcall owner() const override
    {
        return m_owner.GetInterface();
    }

    IDataArray* __stdcall property() const override
    {
        return m_property.GetInterface();
    }

    STATE __stdcall status() const override
    {
        return m_state;
    }

public:
    HRESULT __stdcall AttachMessage
    (
        U8CSTR msg, IComponent* listener
    ) override
    {
        console_out(TEXT("%s::Base::AttachMessage() begin"), NAME);

        if ( nullptr == listener )
        {
            console_out(TEXT("listener is null"));
            console_out(TEXT("%s::Base::AttachMessage() end"), NAME);
            return E_INVALIDARG;
        }

        // デバッグ情報の表示
        console_outA("msg: %s",      (char*)msg);
        console_outA("listener: %s", (char*)listener->name());

        const auto msg_string = std::u8string(msg);
        const auto range = m_msg_map.equal_range(std::move(msg_string));
        const auto end = range.second;
        for ( auto it = range.first; it != end; ++it )
        {
            if ( it->second == listener )
            {
                console_out(TEXT("This message has been already attached to the component"));
                console_out(TEXT("%s::Base::AttachMessage() end"), NAME);
                return S_FALSE;
            }
        }

        m_msg_map.emplace(msg, listener);
        console_out(TEXT("Attachehed"));
        console_out(TEXT("%s::Base::AttachMessage() end"), NAME);

        return S_OK;
    }

    HRESULT __stdcall DetachMessage
    (
        U8CSTR msg, IComponent* listener
    ) override
    {
        console_out(TEXT("%s::Base::DetachMessage() begin"), NAME);

        if ( nullptr == listener )
        {
            console_out(TEXT("listener is null"));
            console_out(TEXT("%s::Base::DetachMessage() end"), NAME);
            return E_INVALIDARG;
        }

        // デバッグ情報の表示
        console_outA("msg: %s",      (char*)msg);
        console_outA("listener: %s", (char*)listener->name());

        const auto msg_string = std::u8string(msg);
        const auto range = m_msg_map.equal_range(std::move(msg_string));
        const auto end = range.second;
        for ( auto it = range.first; it != end; ++it )
        {
            if ( it->second == listener )
            {
                m_msg_map.erase(it);
                console_out(TEXT("Detachehed"));
                console_out(TEXT("%s::Base::DetachMessage() end"), NAME);
                return S_OK;
            }
        }

        console_out(TEXT("This message was not attached to the component"));
        console_out(TEXT("%s::Base::DetachMessage() end"), NAME);

        return S_FALSE;
    }

    HRESULT __stdcall NotifyMessage
    (
        U8CSTR msg, IComponent* sender, IData* data
    ) override
    {
        console_out(TEXT("%s::Base::NotifyMessage() begin"), NAME);

        // デバッグ情報の表示
        console_outA("msg:    %s", (char*)msg);
        console_outA("sender: %s", sender ? (char*)sender->name() : "(null)");
        console_outA("name:   %s",       data ? (char*)data->name() : "(null)");
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
        HWND hwndParent, HWND* phwnd = nullptr
    ) override
    {
        console_out(TEXT("%s:Base::OpenConfiguration() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Start
    (
        void* args = nullptr
    ) override
    {
        console_out(TEXT("%s:Base::Start() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Stop
    (
        void* args = nullptr
    ) override
    {
        console_out(TEXT("%s::Base::Stop() not implemented"), NAME);
        return E_NOTIMPL;
    }

protected:
    HRESULT __stdcall NotifyRequestedMessage
    (
        U8CSTR msg, IData* data
    )
    {
        console_out(TEXT("%s::Base::NotifyRequestedMessage() begin"), NAME);

        // 通知キューにデータを積む
        m_cs_notify.lock();
        {
            NotifyData notify_data { msg, this, data };
            m_q_notify.emplace(notify_data);
        }
        m_cs_notify.unlock();

        // 通知スレッドを起こす
        ::SetEvent(m_evt_notify);

        console_out(TEXT("%s::Base::NotifyRequestedMessage() end"), NAME);

        return S_OK;
    }

private:
    void __stdcall NotifyMessageLoop()
    {
        // よく使う値をローカル定数として記憶
        const auto& evt_notify = this->m_evt_notify;
        auto&       cs_notify  = this->m_cs_notify;
        auto&       q          = this->m_q_notify;

        // メインループ
        NotifyData notify_data;
        while ( m_alive )
        {
            cs_notify.lock();
            {
                if ( q.empty() )
                {
                    notify_data.sender = nullptr;
                }
                else
                {
                    notify_data = q.front();
                    q.pop();
                }
            }
            cs_notify.unlock();

            if ( nullptr == notify_data.sender )
            {
                ::WaitForSingleObject(evt_notify, INFINITE);
            }
            else
            {
                const auto msg    = notify_data.msg;
                const auto sender = notify_data.sender;
                const auto data   = notify_data.data;

                const auto msg_string = std::u8string(msg);
                const auto range = m_msg_map.equal_range(std::move(msg_string));
                const auto end = range.second;
                for ( auto it = range.first; it != end; ++it )
                {
                    const auto listener = it->second.GetInterface();
                    if ( listener )
                    {
                        listener->NotifyMessage(msg, sender, data);
                    }
                }
            }
        }
        while ( ! q.empty() )
        {
            notify_data = q.front();
            q.pop();

            const auto data = notify_data.data;
            if ( data )
            {
                data->Release();
            }
            console_out(TEXT("Discarded notify data"));
        }
    }

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
        IData* data
    ) override
    {
        console_out(TEXT("%s::Base::QuerySupport() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Open
    (
        void* args = nullptr
    ) override
    {
        console_out(TEXT("%s::Base::Open() not implemented"), NAME);
        return E_NOTIMPL;
    }

    HRESULT __stdcall Close
    (
        void* args = nullptr
    ) override
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
    ) final
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
    ) final
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
        size_t offset, void* buffer, size_t buf_size, size_t* cb_data = nullptr
    ) override
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
    ) final
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
        size_t offset, void* buffer, size_t buf_size, size_t* cb_data = nullptr
    ) override
    {
        console_out(TEXT("%s::Base::Write() not implemented"), NAME);
        return E_NOTIMPL;
    }
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// ComponentBase.hpp