// Texture.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// テクスチャオブジェクト
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>

typedef double float64_t;

//---------------------------------------------------------------------------//

// ピクセルフォーマットの内部表現
enum class PixelFormat : uint32_t
{
    UNKNOWN = (uint32_t)-1,
    INDEX8 = 0,
    RGB888,
    BGR888,
    RGBA8888,
    ARGB8888,
    BGRA8888,
    ABGR8888,
};

//---------------------------------------------------------------------------//

// テクスチャ情報を保持する構造体
struct TextureDesc
{
    PixelFormat format;
    int32_t     width;
    int32_t     height;
    float64_t   dpiX;
    float64_t   dpiY;
    int32_t     interpolation_min;
    int32_t     interpolation_max;
    int32_t     repeat_s;
    int32_t     repeat_t;
    int32_t     envi;
};

//---------------------------------------------------------------------------//

// {CB6A5F7A-4599-4758-B027-10446D8DD899}
static const IID IID_ITexture =
{ 0xcb6a5f7a, 0x4599, 0x4758, { 0xb0, 0x27, 0x10, 0x44, 0x6d, 0x8d, 0xd8, 0x99 } };

// テクスチャオブジェクトのためのインターフェイス
interface ITexture : public IUnknown
{
    virtual const void*        __stdcall Buffer()   const = 0;
    virtual const size_t       __stdcall BufSize()  const = 0;
    virtual const TextureDesc* __stdcall Desc()     const = 0;
    virtual const void*        __stdcall Instance() const = 0;

    virtual HRESULT __stdcall Create(const TextureDesc* desc, size_t buf_size, const void* buffer) = 0;
    virtual HRESULT __stdcall Dispose() = 0;
};

//---------------------------------------------------------------------------//

#include <gl/gl.h>

#ifdef THIS
#undef THIS
#endif

#define THIS OpenGLTexture

// ITexure の OpenGL版 実装
class OpenGLTexture : public ITexture
{
public:
    THIS();
    ~THIS();

public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

public:
    const void*        __stdcall Buffer()   const override;
    const size_t       __stdcall BufSize()  const override;
    const TextureDesc* __stdcall Desc()     const override;
    const void*        __stdcall Instance() const override;

    HRESULT __stdcall Create(const TextureDesc* desc, size_t buf_size, const void* buffer) override;
    HRESULT __stdcall Dispose() override;

private:
    ULONG       m_cRef     = 1;
    GLuint      m_instance = 0;
    size_t      m_buf_size = 0;
    uint8_t*    m_buffer   = nullptr;
    TextureDesc m_desc;

private:
    OpenGLTexture(const OpenGLTexture&)             = delete;
    OpenGLTexture(OpenGLTexture&&)                  = delete;
    OpenGLTexture& operator= (const OpenGLTexture&) = delete;
    OpenGLTexture& operator= (OpenGLTexture&&)      = delete;
};

#undef THIS

//---------------------------------------------------------------------------//

// Texture.hpp