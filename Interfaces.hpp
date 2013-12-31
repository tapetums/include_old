// Interfaces.hpp

#pragma once

#pragma execution_character_set("utf-8") 

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

static const IID IID_ICommand =
{ 0xc057dee2, 0xe71a, 0x4bc8, { 0xae, 0x56, 0x75, 0xd4, 0x86, 0x82, 0xfc, 0x6a } };

static const IID IID_ICommandArray =
{ 0xcf0dc445, 0x0697, 0x4490, { 0x8f, 0xa7, 0xbb, 0x92, 0xe4, 0x3c, 0x18, 0xe7 } };

static const IID IID_ICompAdapter =
{ 0x6036103d, 0xe3bd, 0x46ba, { 0xb9, 0xa8, 0x3f, 0xfd, 0xfd, 0x68, 0xd7, 0x23 } };

static const IID IID_ICompCollection =
{ 0x45e0aaf1, 0x3e4e, 0x4e6e, { 0x92, 0x5b, 0x92, 0x9e, 0xc1, 0xa5, 0x63, 0xf7 } };

static const IID IID_IComponent =
{ 0xa35dc0c3, 0xac5f, 0x447a, { 0xa4, 0x60, 0x9c, 0x52, 0x17, 0x72, 0x05, 0x7c } };

static const IID IID_IComponentHost =
{ 0x203d34d8, 0x72f3, 0x4aa4, { 0x9f, 0xf6, 0x13, 0xc2, 0xbd, 0xb4, 0x33, 0xed } };

static const IID IID_ICommandComponent =
{ 0xdb8206c3, 0x3697, 0x4838, { 0x8d, 0xf5, 0xc9, 0x1d, 0x04, 0x97, 0x6f, 0x07 } };

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
interface ICommand;
interface ICommandArray;

interface ICompAdapter;
interface ICompCollection;

interface IComponent;
interface IComponentHost;
interface ICommandComponent;
interface IIOComponent;
interface IReaderComponent;
interface IWriterComponent;

//---------------------------------------------------------------------------//
//
// 文字列定数
//
//---------------------------------------------------------------------------//

// NotifyMessage()メッソドで使われる主なメッセージ
static U8CSTR MSG_NULL = nullptr;

static U8CSTR MSG_COMP_START_DONE   = (U8CSTR)"Comp.Start.Done";
static U8CSTR MSG_COMP_START_FAILED = (U8CSTR)"Comp.Start.Failed";
static U8CSTR MSG_COMP_STOP_DONE    = (U8CSTR)"Comp.Stop.Done";
static U8CSTR MSG_COMP_STOP_FAILED  = (U8CSTR)"Comp.Stop.Failed";

static U8CSTR MSG_IO_CLOSE_DONE     = (U8CSTR)"IO.Close.Done";
static U8CSTR MSG_IO_CLOSE_FAILED   = (U8CSTR)"IO.Close.Failed";
static U8CSTR MSG_IO_OPEN_DONE      = (U8CSTR)"IO.Open.Done";
static U8CSTR MSG_IO_OPEN_FAILED    = (U8CSTR)"IO.Open.Failed";
static U8CSTR MSG_IO_READ_DONE      = (U8CSTR)"IO.Read.Done";
static U8CSTR MSG_IO_READ_FAILED    = (U8CSTR)"IO.Read.Failed";
static U8CSTR MSG_IO_WRITE_DONE     = (U8CSTR)"IO.Write.Done";
static U8CSTR MSG_IO_WRITE_FAILED   = (U8CSTR)"IO.Write.Failed";

//---------------------------------------------------------------------------//

// 言語
static U8CSTR ja_JP = (U8CSTR)"ja-JP";
static U8CSTR en_US = (U8CSTR)"en-US";

//---------------------------------------------------------------------------//
//
// 列挙体
//
//---------------------------------------------------------------------------//

// コンポーネントの種類
enum class COMPTYPE : uint32_t
{
    UNKNOWN  = UINT32_MAX, // 不明なタイプ
    BASIC   = 0,           // 基本コンポーネント
    HOST    = 1,           // コンポーネントホスト
    UI      = 1 << 1,      // UI コンポーネント
    COMMAND = 1 << 2,      // コマンドコンポーネント
    READER  = 1 << 3,      // 入力コンポーネント
    WRITER  = 1 << 4,      // 出力コンポーネント
};

//---------------------------------------------------------------------------//

// コンポーネントの状態
enum class STATE : uint32_t
{
    UNKNOWN  = UINT32_MAX, // 未初期化
    IDLE     = 0,          // 停止中
    ACTIVE   = 1,          // 実行中
    OPEN     = 1 << 1,     // 所有オブジェクトは開かれている
    STARTING = 1 << 2,     // 開始処理中
    STOPPING = 1 << 3,     // 終了処理中
    CLOSING  = 1 << 4,     // 所有オブジェクトを閉じようとしている
    OPENING  = 1 << 5,     // 所有オブジェクトを開こうとしている
    SEEKING  = 1 << 6,     // 所有オブジェクトを走査中
    READING  = 1 << 7,     // 所有オブジェクトからデータを読込中
    WRITING  = 1 << 8,     // 所有オブジェクトにデータを書込中
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
inline bool __stdcall operator ==(const STATE& lhs, const STATE& rhs)
{
    return ((uint32_t)lhs == (uint32_t)rhs);
}
inline bool __stdcall operator !=(const STATE& lhs, const STATE& rhs)
{
    return ((uint32_t)lhs != (uint32_t)rhs);
}

//---------------------------------------------------------------------------//

// コンポーネントの状態を調べるマクロ
#define IS_COMP_BUSY(x) x ? (x->status() >= STATE::OPENING) : false

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
    virtual U8CSTR      __stdcall name()     const = 0;
    virtual size_t      __stdcall size()     const = 0;
    virtual uintptr_t   __stdcall get()      const = 0;
    virtual HRESULT     __stdcall set(uintptr_t value) = 0;
};

//---------------------------------------------------------------------------//

// 汎用データ格納オブジェクト集合のインターフェイス
interface IDataArray : public IUnknown
{
    virtual size_t __stdcall data_count()           const = 0;
    virtual IData* __stdcall data(size_t index = 0) const = 0;
};

//---------------------------------------------------------------------------//

// コンポーネントDLLファイルを透過的に扱うオブジェクトのインターフェイス
interface ICompAdapter : public IUnknown
{
    virtual VerInfo     __stdcall apiver()      const = 0;
    virtual REFCLSID    __stdcall clsid()       const = 0;
    virtual U8CSTR      __stdcall copyright()   const = 0;
    virtual U8CSTR      __stdcall description() const = 0;
    virtual U8CSTR      __stdcall path()        const = 0;
    virtual size_t      __stdcall index()       const = 0;
    virtual IDataArray* __stdcall property()    const = 0;
    virtual U8CSTR      __stdcall name()        const = 0;
    virtual COMPTYPE    __stdcall type()        const = 0;
    virtual VerInfo     __stdcall version()     const = 0;

    virtual HRESULT __stdcall Load(U8CSTR filepath, size_t index) = 0;
    virtual HRESULT __stdcall Free() = 0;
    virtual HRESULT __stdcall CreateInstance(IComponent* owner, REFIID riid, void** ppvObject) = 0;
    virtual HRESULT __stdcall OpenConfiguration(HWND hwndParent, HWND* phwnd) = 0;
};

//---------------------------------------------------------------------------//

typedef bool (__stdcall* CollectIf)(const ICompAdapter* adapter);

// コンポーネント管理オブジェクトのインターフェイス
interface ICompCollection : public IUnknown
{
    virtual size_t        __stdcall size()           const = 0;
    virtual ICompAdapter* __stdcall at(size_t index) const = 0;

    virtual HRESULT          __stdcall Append(U8CSTR path) = 0;
    virtual HRESULT          __stdcall Remove(U8CSTR path) = 0;
    virtual ICompCollection* __stdcall Collect(const CollectIf comparison) = 0;
    virtual ICompAdapter*    __stdcall Find(REFCLSID rclsid) = 0;
};

//---------------------------------------------------------------------------//

// コンポーネントの基本インターフェイス
interface IComponent : public IUnknown
{
    virtual REFCLSID         __stdcall clsid()      const = 0;
    virtual ICompCollection* __stdcall collection() const = 0;
    virtual U8CSTR           __stdcall name()       const = 0;
    virtual IComponent*      __stdcall owner()      const = 0;
    virtual IDataArray*      __stdcall property()   const = 0;
    virtual STATE            __stdcall status()     const = 0;

    virtual HRESULT __stdcall AttachMessage(U8CSTR msg, IComponent* listener) = 0;
    virtual HRESULT __stdcall DetachMessage(U8CSTR msg, IComponent* listener) = 0;
    virtual HRESULT __stdcall NotifyMessage(U8CSTR msg, IComponent* sender, IComponent* listener, IData* data) = 0;
    virtual HRESULT __stdcall Start(void* args, IComponent* listener = nullptr) = 0;
    virtual HRESULT __stdcall Stop (void* args, IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// コマンドコンポーネントのインターフェイス
interface ICommandComponent : public IComponent
{
    virtual HRESULT __stdcall Execute(void* args, IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// 入出力コンポーネントの基底インターフェイス
interface IIOComponent : public IComponent
{
    virtual HRESULT __stdcall Close(IComponent* listener = nullptr) = 0;
    virtual HRESULT __stdcall Open(U8CSTR path, U8CSTR format_as, IComponent* listener = nullptr) = 0;
    virtual HRESULT __stdcall QuerySupport(U8CSTR path, U8CSTR format_as) = 0;
};

//---------------------------------------------------------------------------//

// 入力コンポーネントのインターフェイス
interface IReaderComponent : public IIOComponent
{
    virtual HRESULT __stdcall Read(void* buffer, size_t offset, size_t buf_size, size_t* cb_data, IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

// 出力コンポーネントのインターフェイス
interface IWriterComponent : public IIOComponent
{
    virtual HRESULT __stdcall Write(void* buffer, size_t offset, size_t buf_size, size_t* cb_data, IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Interfaces.hpp