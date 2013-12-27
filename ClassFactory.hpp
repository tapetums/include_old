// ClassFactory.hpp

#pragma once

//---------------------------------------------------------------------------//

extern ULONG g_lock;

//---------------------------------------------------------------------------//

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

// ファクトリクラスの唯一のインスタンス（ClassFactory.cppで定義）
extern CClassFactory ClassFactory;

//---------------------------------------------------------------------------//

// ClassFactory.hpp