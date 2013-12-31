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

// 前方宣言
typedef unsigned char char8_t;

namespace std
{
    template<class Key> struct hash;
    template<class T> struct equal_to;
    template<class T1, class T2> struct pair;
    template<class T> class allocator;
    template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, class Allocator = std::allocator<std::pair<const Key, T>>> class unordered_multimap;

    template<class CharT> struct char_traits;
    template<class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>> class basic_string;

    typedef basic_string<char8_t> u8string;
}

template<typename T> struct ComPtr;
typedef std::unordered_multimap<std::u8string, ComPtr<CubeMelon::IComponent>> MessageMap;

//---------------------------------------------------------------------------//

extern const wchar_t* NAME;

extern const CLSID    CLSID_Component;
extern const char8_t* COMP_NAME;

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
    HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender, IComponent* listener, IData* data) override;
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
    HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender, IComponent* listener, IData* data) override;
    HRESULT __stdcall Start(void* args, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop (void* args, IComponent* listener = nullptr) override;

public:
    HRESULT __stdcall Close(IComponent* listener = nullptr) override;
    HRESULT __stdcall Open(U8CSTR path, U8CSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall QuerySupport(U8CSTR path, U8CSTR format_as) override;

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

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// ComponentBase.hpp