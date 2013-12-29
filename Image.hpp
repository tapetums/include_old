// Image.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>

//---------------------------------------------------------------------------//

// {F6995418-88A6-46E9-8528-64CFDF7B38A7}
static const IID IID_Image = 
{ 0xf6995418, 0x88a6, 0x46e9, { 0x85, 0x28, 0x64, 0xcf, 0xdf, 0x7b, 0x38, 0xa7 } };

interface Image : public IUnknown
{
    virtual BITMAPINFO* __stdcall bmpinfo()    const = 0;
    virtual int32_t     __stdcall width()      const = 0;
    virtual int32_t     __stdcall height()     const = 0;
    virtual uint16_t    __stdcall bitcount()   const = 0;
    virtual uint32_t    __stdcall colorused()  const = 0;
    virtual RGBQUAD*    __stdcall colortable() const = 0;
    virtual uint32_t    __stdcall data_size()  const = 0;
    virtual uint8_t*    __stdcall data()       const = 0;

    virtual HRESULT __stdcall Create(const BITMAPINFO* bmpinfo) = 0;
    virtual HRESULT __stdcall Dispose() = 0;
    virtual Image*  __stdcall Clone() = 0;
};

//---------------------------------------------------------------------------//

class Bitmap : public Image
{
public:
    Bitmap();
    ~Bitmap();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

public:
    BITMAPINFO* __stdcall bmpinfo()    const override;
    int32_t     __stdcall width()      const override;
    int32_t     __stdcall height()     const override;
    uint16_t    __stdcall bitcount()   const override;
    uint32_t    __stdcall colorused()  const override;
    RGBQUAD*    __stdcall colortable() const override;
    uint32_t    __stdcall data_size()  const override;
    uint8_t*    __stdcall data()       const override;

public:
    HRESULT __stdcall Create(const BITMAPINFO* bmpinfo) override;
    HRESULT __stdcall Dispose() override;
    Image*  __stdcall Clone();

public:
    HRESULT __stdcall Load(LPCWSTR filename);
    HRESULT __stdcall Save(LPCWSTR filename);
    HRESULT __stdcall UpsideDown();
    HRESULT __stdcall ToBGRA32();

protected:
    ULONG m_cRef = 0;

private:
    struct Impl;
    Impl* pimpl;

private:
    Bitmap(const Bitmap& lval)             = delete;
    Bitmap(Bitmap&& rval)                  = delete;
    Bitmap& operator =(const Bitmap& lval) = delete;
    Bitmap& operator =(Bitmap&& rval)      = delete;
};

//---------------------------------------------------------------------------//

// Image.hpp
