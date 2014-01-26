// Wav.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// 音声データクラス
//   Copyright (C) 2013-2014 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>
#include <mmreg.h>

//---------------------------------------------------------------------------//

uint32_t __stdcall MaskChannelMask(uint16_t channels);

//---------------------------------------------------------------------------//

// {90350135-D4C2-4C2C-87F2-88BD291D9057}
static const IID IID_IWav = 
{ 0x90350135, 0xd4c2, 0x4c2c, { 0x87, 0xf2, 0x88, 0xbd, 0x29, 0x1d, 0x90, 0x57 } };

interface IWav : public IUnknown
{
    virtual WAVEFORMATEXTENSIBLE* __stdcall format() const = 0;
    virtual LPCWSTR  __stdcall path() const = 0;
    virtual uint64_t __stdcall size() const = 0;
    virtual uint8_t* __stdcall data() const = 0;

    virtual HRESULT __stdcall Create(const WAVEFORMATEXTENSIBLE& format, uint64_t data_size) = 0;
    virtual HRESULT __stdcall Load(LPCWSTR path) = 0;
    virtual HRESULT __stdcall Save(LPCWSTR path) = 0;
    virtual size_t  __stdcall Read (void* buffer, size_t offset, size_t buf_size) = 0;
    virtual size_t  __stdcall Write(void* buffer, size_t offset, size_t buf_size) = 0;
    virtual IWav*   __stdcall Clone() = 0;
    virtual HRESULT __stdcall Dispose() = 0;
};

//---------------------------------------------------------------------------//

class Wav : public IWav
{
public:
    Wav();
    ~Wav();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

public:
    WAVEFORMATEXTENSIBLE* __stdcall format() const override;
    LPCWSTR  __stdcall path() const override;
    uint64_t __stdcall size() const override;
    uint8_t* __stdcall data() const override;

public:
    HRESULT __stdcall Create(const WAVEFORMATEXTENSIBLE& format, uint64_t data_size) override;
    HRESULT __stdcall Load(LPCWSTR path) override;
    HRESULT __stdcall Save(LPCWSTR path) override;
    size_t  __stdcall Read (void* buffer, size_t offset, size_t buf_size) override;
    size_t  __stdcall Write(void* buffer, size_t offset, size_t buf_size) override;
    IWav*   __stdcall Clone() override;
    HRESULT __stdcall Dispose() override;

protected:
    ULONG m_cRef = 0;

private:
    struct Impl;
    Impl* pimpl;

private:
    Wav(const Wav& lval)             = delete;
    Wav(Wav&& rval)                  = delete;
    Wav& operator =(const Wav& lval) = delete;
    Wav& operator =(Wav&& rval)      = delete;
};

//---------------------------------------------------------------------------//

// Wav.hpp