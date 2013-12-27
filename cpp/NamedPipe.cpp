// NamedPipe.cpp

//---------------------------------------------------------------------------//

#include <thread>

#include <stdint.h>
#include <windows.h>
#include <strsafe.h>

#include <DebugPrint.hpp>
#include <GenerateUUIDString.hpp>

#include "NamedPipe.hpp"

//---------------------------------------------------------------------------//
//
// Utility Function
//
//---------------------------------------------------------------------------//

DWORD __stdcall WaitEvents(HANDLE evt1, HANDLE evt2, DWORD dwMilliseconds)
{
    HANDLE evt_array[] = { evt1, evt2, nullptr };

    return ::WaitForMultipleObjects(2, evt_array, FALSE, dwMilliseconds);
}

//---------------------------------------------------------------------------//
//
// Struct for Inner Data
//
//---------------------------------------------------------------------------//

struct NamedPipe::Impl
{
    WCHAR name[MAX_PATH];

    bool is_open = false;
    std::thread thread_connect;
    std::thread thread_wait;

    NamedPipe::DATA data;
};

//---------------------------------------------------------------------------//
//
// Subroutine ( forward declaration )
//
//---------------------------------------------------------------------------//

static HRESULT __stdcall Connect
(
    const NamedPipe::DATA* thread_data,
    NamedPipe::IReadWrite* proceeder,
    LPCWSTR                name
);

//---------------------------------------------------------------------------//
//
// Worker Threads ( forward declaration )
//
//---------------------------------------------------------------------------//

static void __stdcall ThreadWaitClient
(
    const NamedPipe::DATA* thread_data,
    NamedPipe::IReadWrite* proceeder,
    LPCWSTR                name
);

//---------------------------------------------------------------------------//

static void __stdcall ThreadReadWrite
(
    const NamedPipe::DATA* thread_data,
    NamedPipe::IReadWrite* proceeder,
    HANDLE                 evt_thread_begin
);

//---------------------------------------------------------------------------//
//
// Ctor / Dtor
//
//---------------------------------------------------------------------------//

NamedPipe::NamedPipe()
{
    console_out(TEXT("NamedPipe::ctor() begin"));

    pimpl = new Impl;

    pimpl->name[0] = L'\0';
    pimpl->data.evt_close      = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
    pimpl->data.evt_disconnect = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);

    console_out(TEXT("NamedPipe::ctor() end"));
}

//---------------------------------------------------------------------------//

NamedPipe::~NamedPipe()
{
    console_out(TEXT("NamedPipe::dtor() begin"));

    this->Close();

    if ( pimpl->data.evt_disconnect )
    {
        ::CloseHandle(pimpl->data.evt_disconnect);
        pimpl->data.evt_disconnect = nullptr;
    }
    if ( pimpl->data.evt_close )
    {
        ::CloseHandle(pimpl->data.evt_close);
        pimpl->data.evt_close = nullptr;
    }

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("NamedPipe::dtor() end"));
}

//---------------------------------------------------------------------------//
//
// Properties
//
//---------------------------------------------------------------------------//

bool __stdcall NamedPipe::is_open() const
{
    return pimpl->is_open;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall NamedPipe::name() const
{
    return pimpl->name;
}

//---------------------------------------------------------------------------//
//
// Methods
//
//---------------------------------------------------------------------------//

// 新たな名前付きパイプを作成する
HRESULT __stdcall NamedPipe::Create
(
    LPCWSTR     name,
    DWORD       cb_in,
    DWORD       cb_out,
    DWORD       max_instance,
    IReadWrite* proceeder
)
{
    console_out(TEXT("NamedPipe::Create() begin"));

    if ( pimpl->is_open )
    {
        console_out(TEXT("Already created"));
        console_out(TEXT("NamedPipe::Create() end"));
        return S_FALSE;
    }

    // パイプの名前をメンバ変数に記憶
    if ( name )
    {
        // 引数からコピー
        ::StringCchCopyW(pimpl->name, MAX_PATH, name);
    }
    else
    {
        // UUID から生成
        wchar_t buf[MAX_PATH];
        GenerateUUIDStringW(buf, MAX_PATH);
        ::StringCchCopyW(pimpl->name, MAX_PATH, buf);
    }
    console_outW(L"NAME: %s", pimpl->name);

    // その他のデータをメンバ変数に記憶
    pimpl->data.max_instance = max_instance;
    pimpl->data.cb_in        = cb_in;
    pimpl->data.cb_out       = cb_out;

    // 終了フラグをオフ
    ::ResetEvent(pimpl->data.evt_close);

    // クライアント接続待ちスレッドを開始
    pimpl->thread_connect = std::thread
    (
        ThreadWaitClient, &pimpl->data, proceeder, pimpl->name
    );

    // 内部フラグをオンに
    pimpl->is_open = true;

    console_out(TEXT("NamedPipe::Create() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

// 既存の名前付きパイプを開く
HRESULT __stdcall NamedPipe::Open
(
    LPCWSTR     name,
    IReadWrite* proceeder
)
{
    console_out(TEXT("NamedPipe::Open() begin"));

    if ( pimpl->is_open )
    {
        console_out(TEXT("Already opened"));
        console_out(TEXT("NamedPipe::Open() end"));
        return S_FALSE;
    }

    // パイプの名前をメンバ変数に記憶
    ::StringCchCopyW(pimpl->name, MAX_PATH, name);
    console_outW(L"%s", pimpl->name);

    // 終了フラグをオフ
    ::ResetEvent(pimpl->data.evt_close);

    // パイプに接続
    HRESULT hr = Connect(&pimpl->data, proceeder, pimpl->name);

    // 内部フラグをオンに
    pimpl->is_open = true;

    // サーバー空き待ちスレッドを開始
    if ( FAILED(hr) )
    {
        console_out(TEXT("Start waiting until the pipes open"));
        pimpl->thread_wait = std::thread([this, proceeder]() -> void
        {
            WCHAR name_pipe_in[MAX_PATH];
            ::StringCchPrintfW
            (
                name_pipe_in, MAX_PATH, LR"(\\.\pipe\%s_in)", pimpl->name
            );

            while ( true )
            {
                console_out(TEXT("Waiting for pipe open..."));
                auto result = ::WaitNamedPipeW(name_pipe_in, 1000);
                if ( result != 0 )
                {
                    ::Sleep(100); // よくわからないけど先方の処理が終わるまでちょっと待つ
                    auto hr = Connect(&pimpl->data, proceeder, pimpl->name);
                    if ( SUCCEEDED(hr) )
                    {
                        break;
                    }
                }
                if ( ! pimpl->is_open )
                {
                    console_out(TEXT("Closed"));
                    break;
                }
                else
                {
                    console_out(TEXT("Continue to wait"));
                }
            }
        });
        if ( pimpl->thread_wait.joinable() )
        {
            hr = S_FALSE;
        }
    }
    console_out(TEXT("NamedPipe::Open() end"));

    return hr;
}

//---------------------------------------------------------------------------//

// 名前付きパイプを閉じる
HRESULT __stdcall NamedPipe::Close
(
    DWORD dwMilliseconds
)
{
    console_out(TEXT("NamedPipe::Close() begin"));

    ::SetEvent(pimpl->data.evt_close);

    if ( ! pimpl->is_open )
    {
        console_out(TEXT("Already closed"));
        console_out(TEXT("NamedPipe::Close() end"));
        return S_FALSE;
    }

    // 終了フラグをオン
    pimpl->is_open = false;

    // クライアント接続待ちスレッドを終了させる
    if ( pimpl->thread_connect.joinable() )
    {
        pimpl->thread_connect.join();
    }

    // サーバー空き待ちスレッドを終了させる
    if ( pimpl->thread_wait.joinable() )
    {
        pimpl->thread_wait.join();
    }

    // パイプの名前をリセット
    pimpl->name[0] = L'\0';

    console_out(TEXT("NamedPipe::Close() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

// プログラムの終了を待つ
DWORD __stdcall NamedPipe::WaitClose()
{
    auto result = ::WaitForSingleObject(pimpl->data.evt_close, INFINITE);

    if ( pimpl->thread_connect.joinable() )
    {
        pimpl->thread_connect.join();
    }
    if ( pimpl->thread_wait.joinable() )
    {
        pimpl->thread_wait.join();
    }

    return result;
}

//---------------------------------------------------------------------------//
//
// Subroutine
//
//---------------------------------------------------------------------------//

static HRESULT __stdcall Connect
(
    const NamedPipe::DATA* thread_data,
    NamedPipe::IReadWrite* proceeder,
    LPCWSTR                name
)
{
    console_out(TEXT("NamedPipe::Connect() begin"));

    wchar_t name_in[MAX_PATH];
    wchar_t name_out[MAX_PATH];
    ::StringCchPrintfW(name_in,  MAX_PATH, LR"(\\.\pipe\%s_in)",  name);
    ::StringCchPrintfW(name_out, MAX_PATH, LR"(\\.\pipe\%s_out)", name);
    console_outW(L"%s", name_in);
    console_outW(L"%s", name_out);

    // （クライアントから見て）出力パイプに接続
    auto pipe_write = ::CreateFileW
    (
        name_in,
        GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, 0, nullptr
    );

    // （クライアントから見て）入力パイプに接続
    auto pipe_read = ::CreateFileW
    (
        name_out,
        GENERIC_READ,  0, nullptr,
        OPEN_EXISTING, 0, nullptr
    );

    if ( pipe_write == INVALID_HANDLE_VALUE ||
            pipe_read  == INVALID_HANDLE_VALUE )
    {
        console_out(TEXT("Failed to open pipes"));
        console_out(TEXT("NamedPipe::Connect() end"));
        return E_FAIL;
    }

    // パイプの情報を取得
    DWORD cb_write, cb_read;
    ::GetNamedPipeInfo
    (
        pipe_write, nullptr, &cb_write, nullptr, nullptr
    );
    ::GetNamedPipeInfo
    (
        pipe_read, nullptr, &cb_read, nullptr, nullptr
    );

    // 入出力スレッドに渡すデータを準備
    auto data = new NamedPipe::DATA;
    ::memcpy(data, thread_data, sizeof(NamedPipe::DATA));
    data->cb_write   = cb_write;
    data->cb_read    = cb_read;
    data->pipe_write = pipe_write;
    data->pipe_read  = pipe_read;

    // 入出力スレッドを開始
    auto evt_thread_begin = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    auto thread_readwrite = std::thread(ThreadReadWrite, data, proceeder, evt_thread_begin);
    WaitEvents(evt_thread_begin, thread_data->evt_close);

    // 後始末
    ::CloseHandle(evt_thread_begin);
    evt_thread_begin = nullptr;
    thread_readwrite.detach();

    console_out(TEXT("Connected"));

    console_out(TEXT("NamedPipe::Connect() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//
//
// Worker Threads
//
//---------------------------------------------------------------------------//

// クライアント接続待ちスレッド
void __stdcall ThreadWaitClient
(
    const NamedPipe::DATA* thread_data,
    NamedPipe::IReadWrite* proceeder,
    LPCWSTR                name
)
{
    const auto max_instance   = thread_data->max_instance;
    const auto cb_in          = thread_data->cb_in;
    const auto cb_out         = thread_data->cb_out;
    const auto evt_close      = thread_data->evt_close;
    const auto evt_disconnect = thread_data->evt_disconnect;

    // 接続要求イベントの作成
    OVERLAPPED ov = { };
    const auto evt_connect = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if ( nullptr == evt_connect )
    {
        console_out(TEXT("Failed to create connection event"));
        return;
    }
    ov.hEvent = evt_connect;

    // 名前付きパイプ名の生成
    wchar_t name_in[MAX_PATH];
    wchar_t name_out[MAX_PATH];
    ::StringCchPrintfW(name_in,  MAX_PATH, LR"(\\.\pipe\%s_in)",  name);
    ::StringCchPrintfW(name_out, MAX_PATH, LR"(\\.\pipe\%s_out)", name);
    console_outW(L"%s", name_in);
    console_outW(L"%s", name_out);

    DWORD result;
    while ( true )
    {
        // 名前付きパイプのインスタンスを生成
        console_out(TEXT("Creating named pipes..."));
        auto pipe_in = ::CreateNamedPipeW
        (
            name_in,
            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE, max_instance,
            cb_in, cb_in,
            INFINITE, nullptr
        );
        auto pipe_out = ::CreateNamedPipeW
        (
            name_out,
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_BYTE, max_instance,
            cb_out, cb_out,
            INFINITE, nullptr
        );
        if ( pipe_in  == INVALID_HANDLE_VALUE ||
             pipe_out == INVALID_HANDLE_VALUE )
        {
            console_out(TEXT("Waiting for disconnection..."));
            result = WaitEvents(evt_disconnect, evt_close);
            if ( result != WAIT_OBJECT_0 )
            {
                console_out(TEXT("Closed"));
                break;
            }
            console_out(TEXT("Pipe was freed"));
            ::ResetEvent(evt_disconnect);
            continue;
        }
        console_out(TEXT("Created named pipes"));
        console_out(TEXT("pipe_in:  0x%p"), pipe_in);
        console_out(TEXT("pipe_out: 0x%p"), pipe_out);
        console_out(TEXT("Ready to connect"));

        // クライアントからの接続を待つ
        ::ConnectNamedPipe(pipe_in, &ov);
        result = WaitEvents(evt_connect, evt_close);
        if ( result != WAIT_OBJECT_0 )
        {
            ::CloseHandle(pipe_in);
            ::CloseHandle(pipe_out);

            console_out(TEXT("Closed"));
            break;
        }
        ::ConnectNamedPipe(pipe_out, nullptr);
        console_out(TEXT("Connected"));

        auto data = new NamedPipe::DATA;
        ::memcpy(data, thread_data, sizeof(NamedPipe::DATA));
        data->pipe_in  = pipe_in;
        data->pipe_out = pipe_out;

        // 入出力スレッドの開始
        auto evt_thread_begin = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        auto thread_readwrite = std::thread(ThreadReadWrite, data, proceeder, evt_thread_begin);
        WaitEvents(evt_thread_begin, evt_close, 1000);

        // 後始末
        ::CloseHandle(evt_thread_begin);
        evt_thread_begin = nullptr;
        thread_readwrite.detach();
    }

    // 後始末
    ::CloseHandle(evt_connect);

    return;
}

//---------------------------------------------------------------------------//

// 入出力スレッド
void __stdcall ThreadReadWrite
(
    const NamedPipe::DATA* thread_data, 
    NamedPipe::IReadWrite* proceeder,
    HANDLE                 evt_thread_begin
)
{
    ::SetEvent(evt_thread_begin);

    console_out(TEXT("NamedPipe::ThreadReadWrite() begin"));

    // IReadWrite オブジェクトに処理を委譲
    if ( proceeder )
    {
        proceeder->ReadWrite(*thread_data);
    }

    // パイプとの接続を切断
    ::SetEvent(thread_data->evt_disconnect);
    ::DisconnectNamedPipe(thread_data->pipe_in);
    ::DisconnectNamedPipe(thread_data->pipe_out);
    ::CloseHandle(thread_data->pipe_in);
    ::CloseHandle(thread_data->pipe_out);

    // 後始末
    delete thread_data;
    thread_data = nullptr;

    console_out(TEXT("NamedPipe::ThreadReadWrite() end"));

    return;
}

//---------------------------------------------------------------------------//

// NamedPipe.cpp