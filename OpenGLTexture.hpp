// OpenGLTexture.hpp

#pragma once

///---------------------------------------------------------------------------//
//
// ITexure の OpenGL版 実装
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <gl/gl.h>

#include "ITexture.hpp"

//---------------------------------------------------------------------------//

#ifdef THIS
#undef THIS
#endif

#define THIS OpenGLTexture

// ITexure の OpenGL版 実装
class OpenGLTexture : public ITexture
{
public:
    THIS(const TextureDesc* desc, size_t buf_size, const void* buffer);
    ~THIS() override;

    const void*        Buffer()   const override;
    const size_t       BufSize()  const override;
    const TextureDesc* Desc()     const override;
    const void*        Instance() const override;

private:
    void Init(const TextureDesc* desc, size_t buf_size, const void* buffer);
    void Uninit();

private:
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

// OpenGLTexture.hpp