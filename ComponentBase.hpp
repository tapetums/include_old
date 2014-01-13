// ComponentBase.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// コンポーネントの基底クラス
//   Copyright (C) 2013 tapetums
//
//  C++ は 直線継承以外の形でインターフェイスを実装すると
//  (e.g. class Hoge : public Interface, public ClassBase {}; )
//  「定義があいまい」コンパイルエラーになるので、
//   仕方なく重複するコードを複数の .cpp ファイルに書いています。
//
//---------------------------------------------------------------------------//

#include "Interfaces.hpp"

//---------------------------------------------------------------------------//
//
// 前方宣言
//
//---------------------------------------------------------------------------//

typedef unsigned char char8_t;

namespace std
{
    template<class Key> struct hash;
    template<class T> struct equal_to;
    template<class T1, class T2> struct pair;
    template<class T> class allocator;
    template<class Key, class T, class Hash = hash<Key>, class KeyEqual = equal_to<Key>, class Allocator = allocator<pair<const Key, T>>> class unordered_multimap;

    template<class CharT> struct char_traits;
    template<class CharT, class Traits = char_traits<CharT>, class Allocator = allocator<CharT>> class basic_string;

    typedef basic_string<char8_t> u8string;

    template<class T, class Alloc = allocator<T>> class deque;
    template<class T, class Container = deque<T>> class queue;
}

template<typename T> struct ComPtr;
typedef std::unordered_multimap<std::u8string, ComPtr<CubeMelon::IComponent>> MessageMap;

typedef std::queue<std::pair<U8CSTR, ComPtr<CubeMelon::IData>>> NotifyQueue;

//---------------------------------------------------------------------------//

extern const TCHAR* NAME;

extern const CLSID    CLSID_Component;
extern const char8_t* COMP_NAME;

//---------------------------------------------------------------------------//
//
// クラス
//
//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

// コンポーネントの基底クラス
class Component : public IComponent
{
public:
    explicit Component(IUnknown* pUnkOuter = nullptr);
    virtual ~Component();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

public:
    REFCLSID         __stdcall clsid()      const override;
    ICompCollection* __stdcall collection() const override;
    U8CSTR           __stdcall name()       const override;
    IComponent*      __stdcall owner()      const override;
    IDataArray*      __stdcall property()   const override;
    STATE            __stdcall status()     const override;

    HRESULT __stdcall AttachMessage(U8CSTR msg, IComponent* listener) override;
    HRESULT __stdcall DetachMessage(U8CSTR msg, IComponent* listener) override;
    HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender= nullptr, IData* data= nullptr) override;
    HRESULT __stdcall OpenConfiguration(HWND hwndParent, HWND* phwnd = nullptr) override;
    HRESULT __stdcall Start(void* args, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop (void* args, IComponent* listener = nullptr) override;

protected:
    ULONG       m_cRef     = 0;
    STATE       m_state    = STATE::UNKNOWN;
    MessageMap* m_msg_map  = nullptr;
    IComponent* m_owner    = nullptr;
    IDataArray* m_property = nullptr;

private:
    Component(const Component&)             = delete;
    Component(Component&&)                  = delete;
    Component& operator =(const Component&) = delete;
    Component& operator =(Component&&)      = delete;
};

//---------------------------------------------------------------------------//

// 入力コンポーネントの基底クラス
class ReaderComponent : public IReaderComponent
{
public:
    explicit ReaderComponent(IUnknown* pUnkOuter = nullptr);
    virtual ~ReaderComponent();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

public:
    REFCLSID         __stdcall clsid()      const override;
    ICompCollection* __stdcall collection() const override;
    U8CSTR           __stdcall name()       const override;
    IComponent*      __stdcall owner()      const override;
    IDataArray*      __stdcall property()   const override;
    STATE            __stdcall status()     const override;

    HRESULT __stdcall AttachMessage(U8CSTR msg, IComponent* listener) override;
    HRESULT __stdcall DetachMessage(U8CSTR msg, IComponent* listener) override;
    HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender= nullptr, IData* data= nullptr) override;
    HRESULT __stdcall OpenConfiguration(HWND hwndParent, HWND* phwnd = nullptr) override;
    HRESULT __stdcall Start(void* args, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop (void* args, IComponent* listener = nullptr) override;

public:
    HRESULT __stdcall QuerySupport(U8CSTR path, U8CSTR format_as) override;
    HRESULT __stdcall Open(U8CSTR path, U8CSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall Close(IComponent* listener = nullptr) override;

public:
    HRESULT __stdcall Read(void* buffer, size_t offset, size_t buf_size, size_t* cb_data, IComponent* listener = nullptr) override;

protected:
    ULONG       m_cRef     = 0;
    STATE       m_state    = STATE::UNKNOWN;
    MessageMap* m_msg_map  = nullptr;
    IComponent* m_owner    = nullptr;
    IDataArray* m_property = nullptr;

private:
    ReaderComponent(const ReaderComponent&)             = delete;
    ReaderComponent(ReaderComponent&&)                  = delete;
    ReaderComponent& operator =(const ReaderComponent&) = delete;
    ReaderComponent& operator =(ReaderComponent&&)      = delete;
};

//---------------------------------------------------------------------------//

// 出力コンポーネントの基底クラス
class WriterComponent : public IWriterComponent
{
public:
    explicit WriterComponent(IUnknown* pUnkOuter = nullptr);
    virtual ~WriterComponent();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

public:
    REFCLSID         __stdcall clsid()      const override;
    ICompCollection* __stdcall collection() const override;
    U8CSTR           __stdcall name()       const override;
    IComponent*      __stdcall owner()      const override;
    IDataArray*      __stdcall property()   const override;
    STATE            __stdcall status()     const override;

    HRESULT __stdcall AttachMessage(U8CSTR msg, IComponent* listener) override;
    HRESULT __stdcall DetachMessage(U8CSTR msg, IComponent* listener) override;
    HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender= nullptr, IData* data= nullptr) override;
    HRESULT __stdcall OpenConfiguration(HWND hwndParent, HWND* phwnd = nullptr) override;
    HRESULT __stdcall Start(void* args, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop (void* args, IComponent* listener = nullptr) override;

public:
    HRESULT __stdcall QuerySupport(U8CSTR path, U8CSTR format_as) override;
    HRESULT __stdcall Open(U8CSTR path, U8CSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall Close(IComponent* listener = nullptr) override;

public:
    HRESULT __stdcall Write(void* buffer, size_t offset, size_t buf_size, size_t* cb_data, IComponent* listener = nullptr) override;

protected:
    ULONG       m_cRef     = 0;
    STATE       m_state    = STATE::UNKNOWN;
    MessageMap* m_msg_map  = nullptr;
    IComponent* m_owner    = nullptr;
    IDataArray* m_property = nullptr;

private:
    WriterComponent(const WriterComponent&)             = delete;
    WriterComponent(WriterComponent&&)                  = delete;
    WriterComponent& operator =(const WriterComponent&) = delete;
    WriterComponent& operator =(WriterComponent&&)      = delete;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// ComponentBase.hpp