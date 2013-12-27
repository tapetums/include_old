// Susie.hpp

#pragma once

///---------------------------------------------------------------------------//
//
// Susie プラグイン API 定義ファイル (in C++11)
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//
//
// 下記を参考 :
//
// Susie Plug-in Programming Guide
// http://home.netyou.jp/cc/susumu/progSusie.html
//
// Susie Plug-in Specification Rev4+α on Win32
// http://www2f.biglobe.ne.jp/~kana/spi_api/index.html
//
// TORO's Software library
// http://homepage1.nifty.com/toro/slplugin.html
//
// Susie Plug-inプログラミング
// http://www.asahi-net.or.jp/~kh4s-smz/spi/index.html
//
// DでSusieプラグインを使うためのクラス(リンク切れ):
// http://moephp.org/?D%B8%C0%B8%EC#content_1_13
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>

//---------------------------------------------------------------------------//

extern "C" {

//---------------------------------------------------------------------------//
//
// 定数
//
//---------------------------------------------------------------------------//

// SusieFileInfo::path における最大要素数
#define SPI_MAX_PATH 200

// IsSupported()関数へ渡すデータのサイズ
// 余白部分は <必ず> 0 で埋めること!!
#define SPI_TEST_BUF_SIZE 2048

//---------------------------------------------------------------------------//
//
// 列挙体
//
//---------------------------------------------------------------------------//

// Susie エラーコード
enum SPI_RESULT : int32_t
{
    SPI_UNSUPPORTED      =  0, // 非対応のファイル形式
    SPI_SUPPORTED        =  1, // 対応可能なファイル形式

    SPI_NO_FUNCTION      = -1, // その機能はインプリメントされていない
    SPI_ALL_RIGHT        =  0, // 正常終了
    SPI_E_ABORT          =  1, // コールバック関数が非0を返したので展開を中止した
    SPI_NOT_SUPPORT      =  2, // 未知のフォーマット
    SPI_OUT_OF_ORDER     =  3, // データが壊れている
    SPI_NO_MEMORY        =  4, // メモリーが確保出来ない
    SPI_MEMORY_ERROR     =  5, // メモリーエラー
    SPI_FILE_READ_ERROR  =  6, // ファイルリードエラー
    SPI_WINDOW_ERROR     =  7, // 窓が開けない (非公開のエラーコード)
    SPI_OTHER_ERROR      =  8, // 内部エラー
    SPI_FILE_WRITE_ERROR =  9, // 書き込みエラー (非公開のエラーコード)
    SPI_END_OF_FILE      = 10, // ファイル終端 (非公開のエラーコード)
};

//---------------------------------------------------------------------------//

// Susie 追加情報フラグ
enum SPI_FLAG : uint32_t
{
    SPI_INPUT_FILE    = 0,      // 入力はディスクファイル
    SPI_INPUT_MEMORY  = 1,      // 入力はメモリ上のイメージ
    SPI_IGNORE_CASE   = 1 << 7, // ファイル名の大文字小文字を同一視する
    SPI_OUTPUT_FILE   = 0,      // 出力はディスクファイル
    SPI_OUTPUT_MEMORY = 1 << 8, // 出力はメモリ上のイメージ
};

//---------------------------------------------------------------------------//

// Susie 設定ダイアログ 機能コード
enum SPI_FNC_CODE : int32_t
{
    SPI_CONFIGDLG_ABOUT    = 0, // About ダイアログを表示する
    SPI_CONFIGDLG_SETTING  = 1, // 設定ダイアログを表示する
    SPI_CONFIGDLG_RESERVED = 2, // 予約済み
};

//---------------------------------------------------------------------------//
//
// 構造体
//
//---------------------------------------------------------------------------//

#pragma pack(push, 1)

//---------------------------------------------------------------------------//

// Susie 画像情報
struct SusiePictureInfo
{
    int32_t   left, top;  // 画像を展開する位置
    int32_t   width;      // 画像の幅(pixel)
    int32_t   height;     // 画像の高さ(pixel)
    uint16_t  x_density;  // 画素の水平方向密度
    uint16_t  y_density;  // 画素の垂直方向密度
    int16_t   colorDepth; // 画素当たりのbit数
    #if defined(_WIN64) || defined(WIN64)
      uint8_t dummy[2];   // アラインメント
    #endif
    HLOCAL    hInfo;      // 画像内のテキスト情報
};

//---------------------------------------------------------------------------//

// Susie 独自の time_t 型
#if defined(_WIN64) || defined(WIN64)
    typedef int64_t susie_time_t;
#else
    typedef int32_t susie_time_t;
#endif

//---------------------------------------------------------------------------//

// Susie ファイル情報 (ANSI)
struct SusieFileInfoA
{
    uint8_t      method[8];              // 圧縮法の種類
    size_t       position;               // ファイル上での位置
    size_t       compsize;               // 圧縮されたサイズ
    size_t       filesize;               // 元のファイルサイズ
    susie_time_t timestamp;              // ファイルの更新日時
    char         path[SPI_MAX_PATH];     // 相対パス
    char         filename[SPI_MAX_PATH]; // ファイルネーム
    uint32_t     crc;                    // CRC32
    #if defined(_WIN64) || defined(WIN64)
      uint8_t    dummy[4];               // アラインメント
    #endif
};

//---------------------------------------------------------------------------//

// Susie ファイル情報 (Windows UNICODE)
struct SusieFileInfoW
{
    uint8_t      method[8];          // 圧縮法の種類
    size_t       position;           // ファイル上での位置
    size_t       compsize;           // 圧縮されたサイズ
    size_t       filesize;           // 元のファイルサイズ
    susie_time_t timestamp;          // ファイルの更新日時
    wchar_t      path[SPI_MAX_PATH]; // 相対パス
    wchar_t      filename[MAX_PATH]; // ファイルネーム
    uint32_t     crc;                // CRC32
};

//---------------------------------------------------------------------------//

#if defined(_UNICODE) || defined(UNICODE)
    #define SusieFileInfo SusieFileInfoW
#else
    #define SusieFileInfo SusieFileInfoA
#endif

//---------------------------------------------------------------------------//

#pragma pack(pop)

//---------------------------------------------------------------------------//
//
// 関数ポインタ型
//
//---------------------------------------------------------------------------//

// コールバック関数
typedef SPI_RESULT (__stdcall* SUSIE_PROGRESS)(int32_t,int32_t, intptr_t);

//---------------------------------------------------------------------------//

// 共通関数
typedef SPI_RESULT (__stdcall* SPI_GetPluginInfoA)(int32_t, LPSTR,  int32_t);
typedef SPI_RESULT (__stdcall* SPI_GetPluginInfoW)(int32_t, LPWSTR, int32_t);
typedef SPI_RESULT (__stdcall* SPI_IsSupportedA)(LPCSTR,  void*);
typedef SPI_RESULT (__stdcall* SPI_IsSupportedW)(LPCWSTR, void*);

//---------------------------------------------------------------------------//

// '00IN'の関数
typedef SPI_RESULT (__stdcall* SPI_GetPictureInfoA)(LPCSTR,  size_t, SPI_FLAG, SusiePictureInfo*);
typedef SPI_RESULT (__stdcall* SPI_GetPictureInfoW)(LPCWSTR, size_t, SPI_FLAG, SusiePictureInfo*);
typedef SPI_RESULT (__stdcall* SPI_GetPictureA)(LPCSTR,  size_t, SPI_FLAG, HANDLE*, HANDLE*, SUSIE_PROGRESS, intptr_t);
typedef SPI_RESULT (__stdcall* SPI_GetPictureW)(LPCWSTR, size_t, SPI_FLAG, HANDLE*, HANDLE*, SUSIE_PROGRESS, intptr_t);
typedef SPI_RESULT (__stdcall* SPI_GetPreviewA)(LPCSTR,  size_t, SPI_FLAG, HANDLE*, HANDLE*, SUSIE_PROGRESS, intptr_t);
typedef SPI_RESULT (__stdcall* SPI_GetPreviewW)(LPCWSTR, size_t, SPI_FLAG, HANDLE*, HANDLE*, SUSIE_PROGRESS, intptr_t);

//---------------------------------------------------------------------------//

// '00AM'の関数
typedef SPI_RESULT (__stdcall* SPI_GetArchiveInfoA)(LPCSTR,  size_t, SPI_FLAG, HLOCAL*);
typedef SPI_RESULT (__stdcall* SPI_GetArchiveInfoW)(LPCWSTR, size_t, SPI_FLAG, HLOCAL*);
typedef SPI_RESULT (__stdcall* SPI_GetFileInfoA)(LPCSTR,  size_t, LPCSTR,  SPI_FLAG, SusieFileInfoA*);
typedef SPI_RESULT (__stdcall* SPI_GetFileInfoW)(LPCWSTR, size_t, LPCWSTR, SPI_FLAG, SusieFileInfoW*);
typedef SPI_RESULT (__stdcall* SPI_GetFileA)(LPCSTR,  size_t, LPSTR,  SPI_FLAG, SUSIE_PROGRESS, intptr_t);
typedef SPI_RESULT (__stdcall* SPI_GetFileW)(LPCWSTR, size_t, LPWSTR, SPI_FLAG, SUSIE_PROGRESS, intptr_t);

//---------------------------------------------------------------------------//

// オプション関数（Susie v0.40 以降）
typedef SPI_RESULT (__stdcall* SPI_ConfigurationDlg)(HWND, SPI_FNC_CODE);

//---------------------------------------------------------------------------//
//
// Susie 外部公開関数
//
//---------------------------------------------------------------------------//

#define SUSIE_EXPORT extern "C" __declspec(dllexport) SPI_RESULT __stdcall

//---------------------------------------------------------------------------//

SUSIE_EXPORT GetPluginInfo (int32_t infono, LPSTR  buf, int32_t buflen);
SUSIE_EXPORT GetPluginInfoW(int32_t infono, LPWSTR buf, int32_t buflen);

SUSIE_EXPORT IsSupported (LPCSTR  filename, void* dw);
SUSIE_EXPORT IsSupportedW(LPCWSTR filename, void* dw);

SUSIE_EXPORT GetPictureInfo (LPCSTR  buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo);
SUSIE_EXPORT GetPictureInfoW(LPCWSTR buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo);

SUSIE_EXPORT GetPicture (LPCSTR  buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData);
SUSIE_EXPORT GetPictureW(LPCWSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData);

SUSIE_EXPORT GetPreview (LPCSTR  buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData);
SUSIE_EXPORT GetPreviewW(LPCWSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData);

SUSIE_EXPORT GetArchiveInfo (LPCSTR  buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf);
SUSIE_EXPORT GetArchiveInfoW(LPCWSTR buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf);

SUSIE_EXPORT GetFileInfo (LPCSTR  buf, size_t len, LPCSTR  filename, SPI_FLAG flag, SusieFileInfoA* lpInfo);
SUSIE_EXPORT GetFileInfoW(LPCWSTR buf, size_t len, LPCWSTR filename, SPI_FLAG flag, SusieFileInfoW* lpInfo);

SUSIE_EXPORT GetFile (LPCSTR  src, size_t len, LPSTR  dest, SPI_FLAG flag, SUSIE_PROGRESS progressCallback, intptr_t lData);
SUSIE_EXPORT GetFileW(LPCWSTR src, size_t len, LPWSTR dest, SPI_FLAG flag, SUSIE_PROGRESS progressCallback, intptr_t lData);

SUSIE_EXPORT ConfigurationDlg(HWND parent, SPI_FNC_CODE fnc);

//---------------------------------------------------------------------------//
//
// Susie コールバック関数
//
//---------------------------------------------------------------------------//

#define SUSIE_API SPI_RESULT __stdcall

//---------------------------------------------------------------------------//

// コールバック用ダミー関数
// （プラグインの中には progressCallback に nullptr を渡すと落ちるものがあるため）
static SUSIE_API SusieCallbackDummy(int32_t nNum,int32_t nDenom, intptr_t lData)
{
    return SPI_ALL_RIGHT;
}

//---------------------------------------------------------------------------//

} // extern "C"

//---------------------------------------------------------------------------//
//
// C++クラス
//
//---------------------------------------------------------------------------//

// Susie プラグインのラッパークラス
class Susie
{
public:
    Susie();
    ~Susie();

public:
    Susie(Susie&& rhs);
    Susie& operator= (Susie&& rhs);

public:
    LPCTSTR __stdcall SpiPath() const;

public:
    bool __stdcall Load(LPCTSTR spi_path);
    bool __stdcall Free();

    SUSIE_API GetPluginInfo(int32_t infono, LPTSTR buf, int32_t buflen);
    SUSIE_API IsSupported(LPCTSTR filename, void* dw);

    SUSIE_API GetPictureInfo(LPCTSTR buf, size_t len, SPI_FLAG flag, SusiePictureInfo* lpInfo);
    SUSIE_API GetPicture(LPCTSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData);
    SUSIE_API GetPreview(LPCTSTR buf, size_t len, SPI_FLAG flag, HANDLE* pHBInfo, HANDLE* pHBm, SUSIE_PROGRESS progressCallback, intptr_t lData);

    SUSIE_API GetArchiveInfo(LPCTSTR buf, size_t len, SPI_FLAG flag, HLOCAL* lphInf);
    SUSIE_API GetFileInfo(LPCTSTR buf, size_t len, LPCTSTR filename, SPI_FLAG flag, SusieFileInfo* lpInfo);
    SUSIE_API GetFile(LPCTSTR src, size_t len, LPTSTR dest, SPI_FLAG flag, SUSIE_PROGRESS progressCallback, intptr_t lData);

    SUSIE_API ConfigurationDlg(HWND parent, SPI_FNC_CODE fnc);

private:
    TCHAR                m_spi_path[MAX_PATH];
    HMODULE              m_module           = nullptr;
    SPI_GetPluginInfoA   m_GetPluginInfoA   = nullptr;
    SPI_GetPluginInfoW   m_GetPluginInfoW   = nullptr;
    SPI_IsSupportedA     m_IsSupportedA     = nullptr;
    SPI_IsSupportedW     m_IsSupportedW     = nullptr;
    SPI_GetPictureInfoA  m_GetPictureInfoA  = nullptr;
    SPI_GetPictureInfoW  m_GetPictureInfoW  = nullptr;
    SPI_GetPictureA      m_GetPictureA      = nullptr;
    SPI_GetPictureW      m_GetPictureW      = nullptr;
    SPI_GetPreviewA      m_GetPreviewA      = nullptr;
    SPI_GetPreviewW      m_GetPreviewW      = nullptr;
    SPI_GetArchiveInfoA  m_GetArchiveInfoA  = nullptr;
    SPI_GetArchiveInfoW  m_GetArchiveInfoW  = nullptr;
    SPI_GetFileInfoA     m_GetFileInfoA     = nullptr;
    SPI_GetFileInfoW     m_GetFileInfoW     = nullptr;
    SPI_GetFileA         m_GetFileA         = nullptr;
    SPI_GetFileW         m_GetFileW         = nullptr;
    SPI_ConfigurationDlg m_ConfigurationDlg = nullptr;

private:
    Susie(const Susie& lhs)             = delete;
    Susie& operator= (const Susie& lhs) = delete;
};

//---------------------------------------------------------------------------//

// Susie プラグイン管理クラス
class SpiManager
{
public:
    SpiManager();
    ~SpiManager();

public:
    void __stdcall Append(LPCTSTR spi_path);
    void __stdcall Remove(LPCTSTR spi_path);

public:
    size_t __stdcall SpiCount()                                    const;
    Susie* __stdcall GetAt(size_t index)                           const;
    Susie* __stdcall QueryAvailableSpi(LPCTSTR filename, void* dw) const;

private:
    struct Impl;
    Impl* pimpl;

private:
    SpiManager(const SpiManager&)             = delete;
    SpiManager(SpiManager&&)                  = delete;
    SpiManager& operator= (const SpiManager&) = delete;
    SpiManager& operator= (SpiManager&&)      = delete;
};

//---------------------------------------------------------------------------//

// Susie.hpp