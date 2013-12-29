// ClassFactory.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <windows.h>

#include "LockModule.h"
#include "ComPtr.hpp"

//---------------------------------------------------------------------------//

template <class T>
class CClassFactory : public IClassFactory
{
public:
    CClassFactory()  = default;
    ~CClassFactory() = default;

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall LockServer(BOOL fLock) override;

private:
    CClassFactory(const CClassFactory&)             = delete;
    CClassFactory(CClassFactory&&)                  = delete;
    CClassFactory& operator =(const CClassFactory&) = delete;
    CClassFactory& operator =(CClassFactory&&)      = delete;
};

//---------------------------------------------------------------------------//

template <class T> STDMETHODIMP CClassFactory<T>::QueryInterface
(
    REFIID riid,
    void** ppvObject
)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory) )
    {
        *ppvObject = static_cast<IClassFactory*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    return S_OK;
}

//---------------------------------------------------------------------------//

template <class T> STDMETHODIMP_(ULONG) CClassFactory<T>::AddRef()
{
    this->LockServer(TRUE);

    return 2;
}

//---------------------------------------------------------------------------//

template <class T> STDMETHODIMP_(ULONG) CClassFactory<T>::Release()
{
    this->LockServer(FALSE);

    return 1;
}

//---------------------------------------------------------------------------//

template<class T> STDMETHODIMP CClassFactory<T>::CreateInstance
(
     IUnknown* pUnkOuter,
     REFIID    riid,
     void**    ppvObject
)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    ComPtr<T> comp;

    comp = new T(pUnkOuter);
    if ( nullptr == comp )
    {
        return E_OUTOFMEMORY;
    }

    return comp->QueryInterface(riid, ppvObject);
}

//---------------------------------------------------------------------------//

template <class T> STDMETHODIMP CClassFactory<T>::LockServer
(
    BOOL bLock
)
{
    if ( bLock )
    {
        LockModule();
    }
    else
    {
        UnlockModule();
    }

     return S_OK;
}

//---------------------------------------------------------------------------//

// ClassFactory.hpp