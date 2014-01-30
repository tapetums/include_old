// Interfaces.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// CubeMelon コンポーネント API ヘッダー
//   Copyright (C) 2010-2013 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>

//---------------------------------------------------------------------------//

typedef unsigned char char8_t;

#define U8STR  char8_t*
#define U8CSTR const char8_t*

//---------------------------------------------------------------------------//
//
// インターフェイスID
//
//---------------------------------------------------------------------------//

static const IID IID_IData =
{ 0x4fe37727, 0x3644, 0x43bf, { 0x9f, 0xea, 0x3e, 0xd2, 0x48, 0x35, 0x3d, 0xc5 } };

static const IID IID_IDataArray =
{ 0x704e6fe9, 0xb308, 0x4bdf, { 0xb4, 0xa1, 0xd6, 0xaf, 0x95, 0xeb, 0x60, 0xd5 } };

static const IID IID_ICompAdapter =
{ 0x6036103d, 0xe3bd, 0x46ba, { 0xb9, 0xa8, 0x3f, 0xfd, 0xfd, 0x68, 0xd7, 0x23 } };

static const IID IID_ICompAdapterCollection =
{ 0x45e0aaf1, 0x3e4e, 0x4e6e, { 0x92, 0x5b, 0x92, 0x9e, 0xc1, 0xa5, 0x63, 0xf7 } };

static const IID IID_IComponent =
{ 0xa35dc0c3, 0xac5f, 0x447a, { 0xa4, 0x60, 0x9c, 0x52, 0x17, 0x72, 0x05, 0x7c } };

static const IID IID_IIOComponent =
{ 0xeefb9211, 0x0fa4, 0x4c0c, { 0x99, 0xd6, 0xc0, 0x0c, 0xc6, 0xc0, 0xe1, 0x01 } };

static const IID IID_IReaderComponent =
{ 0x6b132e6c, 0x4703, 0x4281, { 0x88, 0x8d, 0x36, 0x7c, 0x1b, 0x40, 0xbd, 0x6a } };

static const IID IID_IWriterComponent =
{ 0xba6ffb76, 0xa1cb, 0x439c, { 0x9f, 0x6d, 0xca, 0xd6, 0xd1, 0x8a, 0x8f, 0x3e } };

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//
//
// 前方宣言
//
//---------------------------------------------------------------------------//

enum class STATE : uint32_t;

struct VerInfo;

interface IData;
interface IDataArray;

interface ICompAdapter;
interface ICompAdapterCollection;

interface IComponent;
interface IIOComponent;
interface IReaderComponent;
interface IWriterComponent;

//---------------------------------------------------------------------------//
//
// 文字列定数 (encoding = UTF-8)
//
//---------------------------------------------------------------------------//

// NotifyMessage()メッソドで使われる主なメッセージ
static U8CSTR MSG_NULL = nullptr;

static U8CSTR MSG_SHOW_TEXT         = (U8CSTR)"ShowText";

static U8CSTR MSG_PROP_CHANGING     = (U8CSTR)"Property.Changing";
static U8CSTR MSG_PROP_CHANGED      = (U8CSTR)"Property.Changed";

static U8CSTR MSG_START_ASYNC       = (U8CSTR)"Start.Async";
static U8CSTR MSG_START_DONE        = (U8CSTR)"Start.Done";
static U8CSTR MSG_START_FAILED      = (U8CSTR)"Start.Failed";
static U8CSTR MSG_STOP_ASYNC        = (U8CSTR)"Stop.Async";
static U8CSTR MSG_STOP_DONE         = (U8CSTR)"Stop.Done";
static U8CSTR MSG_STOP_FAILED       = (U8CSTR)"Stop.Failed";

static U8CSTR MSG_IO_CLOSE_ASYNC    = (U8CSTR)"IO.Close.Async";
static U8CSTR MSG_IO_CLOSE_DONE     = (U8CSTR)"IO.Close.Done";
static U8CSTR MSG_IO_CLOSE_FAILED   = (U8CSTR)"IO.Close.Failed";
static U8CSTR MSG_IO_OPEN_ASYNC     = (U8CSTR)"IO.Open.Async";
static U8CSTR MSG_IO_OPEN_DONE      = (U8CSTR)"IO.Open.Done";
static U8CSTR MSG_IO_OPEN_FAILED    = (U8CSTR)"IO.Open.Failed";
static U8CSTR MSG_IO_READ_ASYNC     = (U8CSTR)"IO.Read.Async";
static U8CSTR MSG_IO_READ_DONE      = (U8CSTR)"IO.Read.Done";
static U8CSTR MSG_IO_READ_FAILED    = (U8CSTR)"IO.Read.Failed";
static U8CSTR MSG_IO_WRITE_ASYNC    = (U8CSTR)"IO.Write.Async";
static U8CSTR MSG_IO_WRITE_DONE     = (U8CSTR)"IO.Write.Done";
static U8CSTR MSG_IO_WRITE_FAILED   = (U8CSTR)"IO.Write.Failed";

//---------------------------------------------------------------------------//

// 言語 (ISO 639-1 / ISO 3166)
static U8CSTR ja_JP = (U8CSTR)"ja-JP";
static U8CSTR en_US = (U8CSTR)"en-US";
static U8CSTR en_UK = (U8CSTR)"en-UK";
static U8CSTR es_ES = (U8CSTR)"es-ES";
static U8CSTR de_DE = (U8CSTR)"de-DE";
static U8CSTR fr_FR = (U8CSTR)"fr-FR";
static U8CSTR pt_PT = (U8CSTR)"pt-PT";
static U8CSTR pt_BR = (U8CSTR)"pt-BR";
static U8CSTR ru_RU = (U8CSTR)"ru-RU";
static U8CSTR it_IT = (U8CSTR)"it-IT";
static U8CSTR zh_CN = (U8CSTR)"zh-CN";
static U8CSTR zh_TW = (U8CSTR)"zh-TW";
static U8CSTR ko_KR = (U8CSTR)"ko-KR";

//---------------------------------------------------------------------------//
//
// 列挙体
//
//---------------------------------------------------------------------------//

// コンポーネントの種類
enum class COMPTYPE : uint32_t
{
    UNKNOWN   = UINT32_MAX, // 不明なタイプ
    BASIC     = 0,          // 基本コンポーネント
    HOST      = 1,          // コンポーネントホスト
    UI        = 1 << 1,     // UI コンポーネント
    COMMAND   = 1 << 2,     // コマンドコンポーネント
    READER    = 1 << 3,     // 入力コンポーネント
    WRITER    = 1 << 4,     // 出力コンポーネント
};

//---------------------------------------------------------------------------//

// コンポーネントの状態
enum class STATE : uint32_t
{
    UNKNOWN   = UINT32_MAX, // 未初期化
    IDLE      = 0,          // 停止中
    ACTIVE    = 1,          // 実行中
    OPEN      = 1 <<  1,    // 所有オブジェクトは開かれている
    BUSY      = 1 << 16,    // （何らかの）処理中
    MESSAGING = 1 << 16,    // メッセージ処理中
    STARTING  = 1 << 17,    // 開始処理中
    STOPPING  = 1 << 18,    // 終了処理中
    CLOSING   = 1 << 19,    // 所有オブジェクトを閉じようとしている
    OPENING   = 1 << 20,    // 所有オブジェクトを開こうとしている
    SEEKING   = 1 << 21,    // 所有オブジェクトを走査中
    READING   = 1 << 22,    // 所有オブジェクトからデータを読込中
    WRITING   = 1 << 23,    // 所有オブジェクトにデータを書込中
};

// 暗黙の変換がなされないので以下の演算子オーバーロードが必要
inline STATE __stdcall operator &(const STATE& lhs, const STATE& rhs)
{
    return (STATE)((uint32_t)lhs & (uint32_t)rhs);
}
inline STATE __stdcall operator |(const STATE& lhs, const STATE& rhs)
{
    return (STATE)((uint32_t)lhs | (uint32_t)rhs);
}
inline STATE __stdcall operator ^(const STATE& lhs, const STATE& rhs)
{
    return (STATE)((uint32_t)lhs ^ (uint32_t)rhs);
}
inline STATE __stdcall operator ~(const STATE& lhs)
{
    return (STATE)(~(uint32_t)lhs);
}
inline STATE& __stdcall operator &=(STATE& lhs, const STATE& rhs)
{
    lhs = lhs & rhs; return lhs;
}
inline STATE& __stdcall operator |=(STATE& lhs, const STATE& rhs)
{
    lhs = lhs | rhs; return lhs;
}
inline STATE& __stdcall operator ^=(STATE& lhs, const STATE& rhs)
{
    lhs = lhs ^ rhs; return lhs;
}

// コンポーネントの状態を調べるマクロ
#define IS_COMP_BUSY(comp) comp ? (comp->status() >= STATE::BUSY) : false

//---------------------------------------------------------------------------//
//
// 構造体
//
//---------------------------------------------------------------------------//

#pragma pack(push, 1)

// バージョン情報
struct VerInfo
{
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    char8_t stage;
};

// 通知データ
struct NotifyData
{
    U8CSTR      msg;
    IComponent* sender;
    IData*      data;
};

#pragma pack(pop)

//---------------------------------------------------------------------------//

// API のバージョン
static const VerInfo API_VERSION { 1, 0, 0, 'a' };

//---------------------------------------------------------------------------//
//
// インターフェイス
//
//---------------------------------------------------------------------------//

// 汎用データ格納オブジェクトのインターフェイス
interface IData : public IUnknown
{
    virtual U8CSTR    __stdcall name() const = 0;
    virtual uint64_t  __stdcall size() const = 0;
    virtual uintptr_t __stdcall get()  const = 0;
    virtual HRESULT   __stdcall set(uintptr_t value) = 0;
};

//---------------------------------------------------------------------------//

// 汎用データ格納オブジェクト集合のインターフェイス
interface IDataArray : public IUnknown
{
    virtual IData* __stdcall at(size_t index) const = 0;
    virtual U8CSTR __stdcall name()           const = 0;
    virtual size_t __stdcall size()           const = 0;
};

//---------------------------------------------------------------------------//

// コンポーネントDLLファイルを透過的に扱うオブジェクトのインターフェイス
interface ICompAdapter : public IUnknown
{
    virtual VerInfo     __stdcall apiver()      const = 0;
    virtual U8CSTR      __stdcall copyright()   const = 0;
    virtual U8CSTR      __stdcall description() const = 0;
    virtual size_t      __stdcall index()       const = 0;
    virtual U8CSTR      __stdcall name()        const = 0;
    virtual U8CSTR      __stdcall path()        const = 0;
    virtual IDataArray* __stdcall property()    const = 0;
    virtual REFCLSID    __stdcall rclsid()      const = 0;
    virtual COMPTYPE    __stdcall type()        const = 0;
    virtual VerInfo     __stdcall version()     const = 0;

    virtual HRESULT __stdcall Load(U8CSTR filepath, size_t index) = 0;
    virtual HRESULT __stdcall Free() = 0;
    virtual HRESULT __stdcall CreateInstance(IComponent* owner, REFIID riid, void** ppvObject) = 0;
    virtual HRESULT __stdcall OpenConfiguration(HWND hwndParent, HWND* phwnd = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// コンポーネント管理オブジェクトのインターフェイス
interface ICompAdapterCollection : public IUnknown
{
    typedef bool (__stdcall* Condition)(ICompAdapter* adapter);

    virtual size_t        __stdcall size()           const = 0;
    virtual ICompAdapter* __stdcall at(size_t index) const = 0;

    virtual HRESULT                 __stdcall Append(U8CSTR path) = 0;
    virtual HRESULT                 __stdcall Remove(U8CSTR path) = 0;
    virtual ICompAdapterCollection* __stdcall Collect(Condition condition) = 0;
    virtual ICompAdapter*           __stdcall Find(REFCLSID rclsid) = 0;
};

//---------------------------------------------------------------------------//

// コンポーネントの基本インターフェイス
interface IComponent : public IUnknown
{
    virtual ICompAdapterCollection* __stdcall adapters() const = 0;
    virtual U8CSTR                  __stdcall name()     const = 0;
    virtual IComponent*             __stdcall owner()    const = 0;
    virtual IDataArray*             __stdcall property() const = 0;
    virtual REFCLSID                __stdcall rclsid()   const = 0;
    virtual IComponent*             __stdcall root()     const = 0;
    virtual STATE                   __stdcall status()   const = 0;

    virtual HRESULT __stdcall AttachMessage(U8CSTR msg, IComponent* listener) = 0;
    virtual HRESULT __stdcall DetachMessage(U8CSTR msg, IComponent* listener) = 0;
    virtual HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender, IData* data) = 0;
    virtual HRESULT __stdcall OpenConfiguration(HWND hwndParent, HWND* phwnd = nullptr) = 0;
    virtual HRESULT __stdcall Start(void* args = nullptr) = 0;
    virtual HRESULT __stdcall Stop (void* args = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// 入出力コンポーネントの基底インターフェイス
interface IIOComponent : public IComponent
{
    virtual HRESULT __stdcall QuerySupport(IData* data) = 0;
    virtual HRESULT __stdcall Open (void* args = nullptr) = 0;
    virtual HRESULT __stdcall Close(void* args = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// 入力コンポーネントのインターフェイス
interface IReaderComponent : public IIOComponent
{
    virtual HRESULT __stdcall Read(size_t offset, void* buffer, size_t buf_size, size_t* cb_read = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// 出力コンポーネントのインターフェイス
interface IWriterComponent : public IIOComponent
{
    virtual HRESULT __stdcall Write(size_t offset, void* buffer, size_t buf_size, size_t* cb_written = nullptr) = 0;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Interfaces.hpp