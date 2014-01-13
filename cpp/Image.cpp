// Image.cpp

//---------------------------------------------------------------------------//

#include "DebugPrint.hpp"

#include "Image.hpp"

//---------------------------------------------------------------------------//

class CriticalSection
{
public:
    CriticalSection()  { ::InitializeCriticalSection(&cs); }
    ~CriticalSection() { ::DeleteCriticalSection(&cs); }
    void __stdcall lock()   { ::EnterCriticalSection(&cs); }
    void __stdcall unlock() { ::LeaveCriticalSection(&cs); }

private:
    CRITICAL_SECTION cs;
};

//---------------------------------------------------------------------------//

struct Bitmap::Impl
{
    CriticalSection cs;

    BITMAPINFO* bmpinfo   = nullptr;
    int32_t     width     = 0;
    int32_t     height    = 0;
    uint16_t    bitcount  = 0;
    uint32_t    clrused   = 0;
    int32_t     stride    = 0;
    uint32_t    data_size = 0;
    uint8_t*    data      = nullptr;

    // よくアクセスする情報をメンバ変数にコピー
    void SetVals()
    {
        width    = bmpinfo->bmiHeader.biWidth;
        height   = abs(bmpinfo->bmiHeader.biHeight);
        bitcount = bmpinfo->bmiHeader.biBitCount; 
        stride   = (((width * bitcount) + 31) & ~31) / 8; // 4バイト境界に揃える

        if ( bitcount > 8 )
        {
            clrused = 0;
        }
        else if ( bmpinfo->bmiHeader.biClrUsed > 0 )
        {
            clrused = bmpinfo->bmiHeader.biClrUsed;
        }
        else
        {
            clrused = (1 << bitcount);
        }
    }

    // ピクセルデータ領域を確保
    void AllocDataArea()
    {
        data_size = stride * height;
        data = new uint8_t[data_size];
    }

    // デバッグ情報を表示
    void ShowParameters()
    {
        console_out
        (
            TEXT("(width, height) = (%d, %d)"),
            width, height
        );
        console_out
        (
            TEXT("bit count: %d"), bitcount
        );
        console_out
        (
            TEXT("color used: %d"), clrused
        );
        console_out
        (
            TEXT("stride: %d"), stride
        );
        console_out
        (
            TEXT("Data: %p"), data
        );
        console_out
        (
            TEXT("Data size: %lld bytes"), data_size
        );
    }

    // 内部情報を初期化
    void InitiInternal()
    {
        SetVals();
        AllocDataArea();
        ShowParameters();
    }
};

//---------------------------------------------------------------------------//

Bitmap::Bitmap()
{
    console_out(TEXT("%s::ctor begin"), TEXT(__FILE__));

    pimpl = new Impl;

    this->AddRef();

    console_out(TEXT("%s::ctor end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

Bitmap::~Bitmap()
{
    console_out(TEXT("%s::dtor begin"), TEXT(__FILE__));

    this->Dispose();

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("%s::dtor end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::QueryInterface
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
    else if ( IsEqualIID(riid, IID_Image) )
    {
        console_out(TEXT("IID_Image"));
        *ppvObject = static_cast<Image*>(this);
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

ULONG __stdcall Bitmap::AddRef()
{
    LONG cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::AddRef(): %d -> %d"), TEXT(__FILE__), cRef - 1, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Bitmap::Release()
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

BITMAPINFO* __stdcall Bitmap::bmpinfo() const
{
    return pimpl->bmpinfo;
}

//---------------------------------------------------------------------------//

int32_t __stdcall Bitmap::width() const
{
    return pimpl->width;
}

//---------------------------------------------------------------------------//

int32_t __stdcall Bitmap::height() const
{
    return pimpl->height;
}

//---------------------------------------------------------------------------//

uint16_t __stdcall Bitmap::bitcount() const
{
    return pimpl->bitcount;
}

//---------------------------------------------------------------------------//

uint32_t __stdcall Bitmap::colorused() const
{
    return pimpl->clrused;
}

//---------------------------------------------------------------------------//

RGBQUAD* __stdcall Bitmap::colortable() const
{
    return pimpl->clrused ? (RGBQUAD*)(pimpl->bmpinfo + sizeof(BITMAPINFOHEADER)) : nullptr;
}

//---------------------------------------------------------------------------//

uint32_t __stdcall Bitmap::data_size() const
{
    return pimpl->data_size;
}

//---------------------------------------------------------------------------//

uint8_t* __stdcall Bitmap::data()  const
{
    return pimpl->data;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::Create(const BITMAPINFO* bmpinfo)
{
    console_out(TEXT("%s::Create() begin"), TEXT(__FILE__));

    if ( nullptr == bmpinfo )
    {
        console_out(TEXT("BITMAPINFO* is null"));
        console_out(TEXT("%s::Create() end"), TEXT(__FILE__));
        return E_INVALIDARG;
    }

    // 内部データを初期化
    pimpl->bmpinfo = const_cast<BITMAPINFO*>(bmpinfo);
    pimpl->InitiInternal();

    // ヘッダ情報をコピー
    size_t bmi_size = sizeof(BITMAPINFOHEADER) + (pimpl->clrused * sizeof(RGBQUAD));
    pimpl->bmpinfo = (BITMAPINFO*) new uint8_t[bmi_size];
    ::memcpy(pimpl->bmpinfo, bmpinfo, bmi_size);
    console_out(TEXT("Bitmap header size: %d bytes"), bmi_size);

    console_out(TEXT("%s::Create() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::Dispose()
{
    console_out(TEXT("%s::Dispose() begin"), TEXT(__FILE__));

    pimpl->width     = 0;
    pimpl->height    = 0;
    pimpl->bitcount  = 0;
    pimpl->clrused   = 0;
    pimpl->stride    = 0;
    pimpl->data_size = 0;

    if ( pimpl->data )
    {
        delete[] pimpl->data;
        pimpl->data = nullptr;
    }
    if ( pimpl->bmpinfo )
    {
        delete[] pimpl->bmpinfo;
        pimpl->bmpinfo = nullptr;
    }

    console_out(TEXT("%s::Dispose() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

Image* __stdcall Bitmap::Clone()
{
    console_out(TEXT("%s::Clone() begin"), TEXT(__FILE__));

    auto bitmap = new Bitmap;

    pimpl->cs.lock();
    {
        // ヘッダ情報のコピー
        bitmap->Create(pimpl->bmpinfo);

        // ピクセルデータのコピー
        ::memcpy(bitmap->pimpl->data, pimpl->data, bitmap->pimpl->data_size);
    }
    pimpl->cs.unlock();

    console_out(TEXT("%s::Clone() end"), TEXT(__FILE__));

    return bitmap;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::Load(LPCWSTR filename)
{
    console_out(TEXT("%s::Load() begin"), TEXT(__FILE__));
    console_out(TEXT("%s"), filename);

    if ( pimpl->data )
    {
        console_out(TEXT("Data already exists"));
        this->Dispose();
    }

    const auto hFile = ::CreateFile
    (
        filename, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        console_out(TEXT("CreateFile() failed"), TEXT(__FILE__));
        console_out(TEXT("%s::Load() end"), TEXT(__FILE__));
        return E_FAIL;
    }

    HRESULT          hr;
    DWORD            dwRead;
    BITMAPFILEHEADER bmpfh;

    // ファイルヘッダの読み込み
    ::ReadFile(hFile, &bmpfh, sizeof(BITMAPFILEHEADER), &dwRead, nullptr);
    console_out(TEXT("%d bytes read"), dwRead);

    const auto p = (char*)&bmpfh;
    console_out(TEXT("The first 2 characters: %c%c"), p[0], p[1]);
    if ( bmpfh.bfType != 0x4D42 ) // 0x4D42 = "BM" ( @little endian )
    {
        console_out(TEXT("This is not a bitmap file"));
        hr = E_FAIL;
        goto CLOSE_FILE;
    }

    DWORD bmi_size = bmpfh.bfOffBits - sizeof(BITMAPFILEHEADER);
    console_out(TEXT("Bitmap header size: %d"), bmi_size);

    pimpl->cs.lock();
    {
        // ヘッダ情報およびパレット情報の読み込み ( V4やV5でもこれでOK )
        pimpl->bmpinfo = (BITMAPINFO*) new uint8_t[bmi_size];
        ::ReadFile(hFile, pimpl->bmpinfo, bmi_size, &dwRead, nullptr);
        console_out(TEXT("%d bytes read"), dwRead);

        // 内部データの初期化
        pimpl->InitiInternal();

        // ピクセルデータの読み込み
        ::ReadFile(hFile, pimpl->data, pimpl->data_size, &dwRead, nullptr);
        console_out(TEXT("%d bytes read"), dwRead);
    }
    pimpl->cs.unlock();

    hr = S_OK;

CLOSE_FILE:
    if ( hFile )
    {
        ::CloseHandle(hFile);
    }

    console_out(TEXT("%s::Load() end"), TEXT(__FILE__));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::Save(LPCWSTR filename)
{
    console_out(TEXT("%s::Save() begin"), TEXT(__FILE__));
    console_out(TEXT("%s"), filename);

    if ( nullptr == pimpl->data )
    {
        console_out(TEXT("No data to save"));
        console_out(TEXT("%s::Save() end"), TEXT(__FILE__));
        return S_FALSE;
    }

    const auto hFile = ::CreateFile
    (
        filename, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        console_out(TEXT("CreateFile() failed"), TEXT(__FILE__));
        console_out(TEXT("%s::Save() end"), TEXT(__FILE__));
        return E_FAIL;
    }

    HRESULT hr;
    DWORD   dwWritten;

    pimpl->cs.lock();
    {
        // ファイルヘッダの書き出し
        BITMAPFILEHEADER bmpfh = { };
        bmpfh.bfType      = 0x4D42; // "BM" @ little endian
        bmpfh.bfOffBits   = sizeof(BITMAPFILEHEADER) +
                            pimpl->bmpinfo->bmiHeader.biSize + sizeof(RGBQUAD) * pimpl->clrused;
        bmpfh.bfReserved1 = 0;
        bmpfh.bfReserved2 = 0;
        bmpfh.bfSize      = bmpfh.bfOffBits + pimpl->bmpinfo->bmiHeader.biSizeImage;
        ::WriteFile(hFile, &bmpfh, sizeof(BITMAPFILEHEADER), &dwWritten, nullptr);
        console_out(TEXT("%d bytes wrote"), dwWritten);

        // ヘッダ情報およびパレット情報の書き出し
        ::WriteFile
        (
            hFile, pimpl->bmpinfo,
            pimpl->bmpinfo->bmiHeader.biSize + sizeof(RGBQUAD) * pimpl->clrused,
            &dwWritten, nullptr
        );
        console_out(TEXT("%d bytes wrote"), dwWritten);

        // ピクセルデータの書き出し
        ::WriteFile(hFile, pimpl->data, pimpl->data_size, &dwWritten, nullptr);
        console_out(TEXT("%d bytes wrote"), dwWritten);
    }
    pimpl->cs.unlock();

    hr = S_OK;

    if ( hFile )
    {
        ::CloseHandle(hFile);
    }

    console_out(TEXT("%s::Save() end"), TEXT(__FILE__));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::UpsideDown()
{
    console_out(TEXT("%s::UpsideDown() begin"), TEXT(__FILE__));

    int32_t  height;
    int32_t  stride;
    uint8_t* data;

    pimpl->cs.lock();
    {
        height = pimpl->height;
        stride = pimpl->stride;
        data   = new uint8_t[pimpl->data_size];

        for ( int32_t h = 0; h < height; ++h )
        {
            auto dst = data + h * stride;
            auto src = pimpl->data + (height - h - 1) * stride;
            ::memcpy(dst, src, stride);
        }

        delete[] pimpl->data;

        pimpl->data = data;
        pimpl->bmpinfo->bmiHeader.biHeight *= -1;
    }
    pimpl->cs.unlock();

    console_out(TEXT("%s::UpsideDown() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Bitmap::ToBGRA32()
{
    console_out(TEXT("%s::ToBGRA32() begin"), TEXT(__FILE__));

    int32_t     height;
    int32_t     width;
    uint32_t    data_size;
    uint8_t*    data;
    BITMAPINFO* bmpinfo;

    pimpl->cs.lock();
    {
        // ヘッダ情報のコピー
        const auto bmi_size = pimpl->bmpinfo->bmiHeader.biSize;
        bmpinfo = (BITMAPINFO*) new uint8_t[bmi_size];
        memcpy(bmpinfo, pimpl->bmpinfo, bmi_size);
        bmpinfo->bmiHeader.biSize = bmi_size;
        console_out(TEXT("Bitmap header size: %d bytes"), bmi_size);

        // ピクセルデータの変換
        height    = pimpl->height;
        width     = pimpl->width;
        data_size = 4 * width * height;
        data      = new uint8_t[data_size];

        auto  p = data;
        auto pp = pimpl->data;
        
        if ( pimpl->bitcount == 8 )
        {
            console_out(TEXT("Index color"));
            auto clrtbl = (RGBQUAD*)(&pimpl->bmpinfo->bmiColors);
            while ( p < data + data_size )
            {
                p[0] = clrtbl[*pp].rgbBlue;
                p[1] = clrtbl[*pp].rgbGreen;
                p[2] = clrtbl[*pp].rgbRed;
                p[3] = 0xFF;

                p += 4;
                pp++;
            }
        }
        else if ( pimpl->bitcount == 24 )
        {
            console_out(TEXT("Full color"));
            while ( p < data + data_size )
            {
                p[0] = pp[0];
                p[1] = pp[1];
                p[2] = pp[2];
                p[3] = 0xFF;

                 p += 4;
                pp += 3;
            }
        }
        else
        {
            delete[] data;
            console_out(TEXT("Unsupported bit color"));
            console_out(TEXT("%s::ToBGRA32() end"), TEXT(__FILE__));
            return E_FAIL;
        }

        delete[] pimpl->bmpinfo;
        delete[] pimpl->data;

        pimpl->bmpinfo = bmpinfo;
        pimpl->bmpinfo->bmiHeader.biBitCount  = 32;
        pimpl->bmpinfo->bmiHeader.biClrUsed   = 0;
        pimpl->bmpinfo->bmiHeader.biSizeImage = data_size;
        pimpl->bitcount  = 32;
        pimpl->clrused   = 0;
        pimpl->stride    = 4 * width;
        pimpl->data_size = data_size;
        pimpl->data      = data;
    }
    pimpl->cs.unlock();

    pimpl->ShowParameters();

    console_out(TEXT("%s::ToBGRA32() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

// Image.cpp