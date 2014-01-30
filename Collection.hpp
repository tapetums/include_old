// Collection.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//

static const IID IID_ICollection =
{ 0x3462e383, 0x3071, 0x4e35, { 0x9f, 0xaf, 0xf5, 0x32, 0x82, 0xed, 0x19, 0xd6 } };

template <class I, typename Item, typename Src, typename Key>
interface ICollection : public I
{
    typedef bool (__stdcall* Condition)(Item item);

    virtual ~ICollection() { };

    virtual size_t __stdcall size()           const = 0;
    virtual Item   __stdcall at(size_t index) const = 0;

    virtual HRESULT      __stdcall Append(Src src) = 0;
    virtual HRESULT      __stdcall Remove(Src src) = 0;
    virtual HRESULT      __stdcall Clear() = 0;
    virtual ICollection* __stdcall Collect(Condition condition) = 0;
    virtual Item         __stdcall Find(Key key) = 0;
};

//---------------------------------------------------------------------------//

template <typename Item, typename Src>
interface ICollection<void, Item, Src, void>
{
    typedef bool (__stdcall* Condition)(Item item);

    virtual ~ICollection() { };

    virtual size_t __stdcall size()           const = 0;
    virtual Item   __stdcall at(size_t index) const = 0;

    virtual HRESULT      __stdcall Append(Src src) = 0;
    virtual HRESULT      __stdcall Remove(Src src) = 0;
    virtual HRESULT      __stdcall Clear() = 0;
    virtual ICollection* __stdcall Collect(Condition condition) = 0;
};

template <typename Item, typename Src, typename Key>
interface ICollection<void, Item, Src, Key>
{
    typedef bool (__stdcall* Condition)(Item item);

    virtual ~ICollection() { };

    virtual size_t __stdcall size()           const = 0;
    virtual Item   __stdcall at(size_t index) const = 0;

    virtual HRESULT      __stdcall Append(Src src) = 0;
    virtual HRESULT      __stdcall Remove(Src src) = 0;
    virtual HRESULT      __stdcall Clear() = 0;
    virtual ICollection* __stdcall Collect(Condition condition) = 0;
    virtual Item         __stdcall Find(Key key) = 0;
};

template <class I, typename Item, typename Src, typename Key>
interface ICollection<I, Item, Src, void> : public I
{
    typedef bool (__stdcall* Condition)(Item item);

    virtual ~ICollection() { };

    virtual size_t __stdcall size()           const = 0;
    virtual Item   __stdcall at(size_t index) const = 0;

    virtual HRESULT      __stdcall Append(Src src) = 0;
    virtual HRESULT      __stdcall Remove(Src src) = 0;
    virtual HRESULT      __stdcall Clear() = 0;
    virtual ICollection* __stdcall Collect(Condition condition) = 0;
};

//---------------------------------------------------------------------------//

// Collection.hpp