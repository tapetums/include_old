// OpenGLTexture.cpp

///---------------------------------------------------------------------------//
//
// ITexure の OpenGL版 実装
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include <gl/gl.h>
#include <gl/glext.h>
#pragma comment(lib, "opengl32.lib")

#include "DebugPrint.hpp"

#include "Texture.hpp"

//---------------------------------------------------------------------------//

#ifdef THIS
#undef THIS
#endif

#define THIS OpenGLTexture

//---------------------------------------------------------------------------//

OpenGLTexture::THIS()
{
    console_out(TEXT("%s::ctor"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

OpenGLTexture::~THIS()
{
    console_out(TEXT("%s::dtor begin"), TEXT(__FILE__));

    this->Dispose();

    console_out(TEXT("%s::dtor end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OpenGLTexture::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    console_out(TEXT("%s::QueryInterface() begin"), TEXT(__FILE__));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) )
    {
        console_out(TEXT("IID_IUnknown"));
        *ppvObject = static_cast<IUnknown*>(this);
    }
    else if ( IsEqualIID(riid, IID_ITexture) )
    {
        console_out(TEXT("IID_ITexture"));
        *ppvObject = static_cast<ITexture*>(this);
    }
    else
    {
        console_out(TEXT("No such an interface"));
        console_out(TEXT("%s::QueryInterface() end"), TEXT(__FILE__));
        return E_NOINTERFACE;
    }

    this->AddRef();

    console_out(TEXT("%s::QueryInterface() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall OpenGLTexture::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::AddRef(): %d -> %d"), TEXT(__FILE__), cRef - 1, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall OpenGLTexture::Release()
{
    if ( m_cRef < 1 )
    {
        console_out(TEXT("%s::Release() %d"), TEXT(__FILE__), m_cRef);
        return m_cRef;
    }

    LONG cRef = ::InterlockedDecrement(&m_cRef);

    console_out(TEXT("%s::Release(): %d -> %d"), TEXT(__FILE__), cRef + 1, cRef);

    if ( cRef == 0 )
    {
        console_out(TEXT("%s::delete begin"), TEXT(__FILE__));
        {
            delete this;
        }
        console_out(TEXT("%s::delete end"), TEXT(__FILE__));
    }

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

const void* __stdcall OpenGLTexture::Buffer() const
{
    return m_buffer;
}

//---------------------------------------------------------------------------//

const size_t __stdcall OpenGLTexture::BufSize() const
{
    return m_buf_size;
}

//---------------------------------------------------------------------------//

const TextureDesc* __stdcall OpenGLTexture::Desc() const
{
    return &m_desc;
}

//---------------------------------------------------------------------------//

const void* __stdcall OpenGLTexture::Instance() const
{
    return &m_instance;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OpenGLTexture::Create
(
    const TextureDesc* desc, size_t buf_size, const void* buffer
)
{
    console_out(TEXT("%s::Create() begin"), TEXT(__FILE__));

    m_desc     = *desc;
    m_buf_size = buf_size;
    m_buffer   = new uint8_t[buf_size / sizeof(uint8_t)];
    ::CopyMemory(m_buffer, buffer, buf_size);

    int32_t internal_format = GL_RGBA;
    int32_t format = 0;
    int32_t type   = 0;
    GLint   param  = 0;
    switch( m_desc.format )
    {
        case PixelFormat::INDEX8:
        {
            internal_format = GL_RED;
            format = GL_COLOR_INDEX;
            type   = GL_UNSIGNED_BYTE;
            param  = 1;
            break;
        }
        case PixelFormat::RGB888:
        {
            format = GL_RGB;
            type   = GL_UNSIGNED_BYTE;
            param  = 1;
            break;
        }
        case PixelFormat::BGR888:
        {
            format = GL_BGR;
            type   = GL_UNSIGNED_BYTE;
            param  = 1;
            break;
        }
        case PixelFormat::RGBA8888:
        {
            format = GL_RGBA;
            type   = GL_UNSIGNED_INT_8_8_8_8_REV;
            param  = 4;
            break;
        }
        case PixelFormat::BGRA8888:
        {
            format = GL_BGRA;
            type   = GL_UNSIGNED_INT_8_8_8_8_REV;
            param  = 4;
            break;
        }
        default:
        {
            format = 0;
            type   = 0;
            param  = 0;
            break;
        }
    }

    HRESULT hr;

    // テクスチャを設定
    ::glGenTextures(1, &m_instance);
    if ( m_instance == 0 )
    {
        console_out(TEXT("glGenTextures error: 0x%04x"), glGetError());
        hr = E_FAIL;
    }
    else
    {
        ::glBindTexture(GL_TEXTURE_2D, m_instance);
        ::glPixelStorei(GL_UNPACK_ALIGNMENT, param);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_desc.interpolation_min);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_desc.interpolation_min);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_desc.repeat_s);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_desc.repeat_t);
        ::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        ::glTexImage2D
        (
            GL_TEXTURE_2D, 0, internal_format,
            m_desc.width, m_desc.height, 0,
            format, type, m_buffer
        );
        ::glBindTexture(GL_TEXTURE_2D, 0);
        hr = S_OK;
    }

    console_out(TEXT("%s::Create() end"), TEXT(__FILE__));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OpenGLTexture::Dispose()
{
    console_out(TEXT("%s::Dispose() begin"), TEXT(__FILE__));

    if ( m_instance != 0 )
    {
        ::glDeleteTextures(1, &m_instance);
        m_instance = 0;
    }

    m_buf_size = 0;

    if ( m_buffer )
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }

    ::ZeroMemory(&m_desc, sizeof(m_desc));

    console_out(TEXT("%s::Dispose() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

#undef THIS

//---------------------------------------------------------------------------//

// OpenGLTexture.cpp