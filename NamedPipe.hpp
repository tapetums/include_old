// NamedPipe.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// 名前付きパイプをカプセル化するクラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

// 2つのイベントを待つためのユーティリティ関数
DWORD __stdcall WaitEvents(HANDLE evt1, HANDLE evt2, DWORD dwMilliseconds = INFINITE);

//---------------------------------------------------------------------------//

// 名前付きパイプ
class NamedPipe
{
public:
    struct    DATA;
    interface IReadWrite;

public:
    NamedPipe();
    ~NamedPipe();

public:
    bool    __stdcall is_open() const;
    LPCWSTR __stdcall name()    const;

public:
    HRESULT __stdcall Create(LPCWSTR name, DWORD cb_in, DWORD cb_out, DWORD max_instance, IReadWrite* proceeder);
    HRESULT __stdcall Open(LPCWSTR name, IReadWrite* proceeder);
    HRESULT __stdcall Close(DWORD dwMilliseconds = INFINITE);
    DWORD   __stdcall WaitClose();

private:
    struct Impl;
    Impl* pimpl;

private:
    NamedPipe(const NamedPipe& lhs)             = delete;
    NamedPipe(NamedPipe&& rhs)                  = delete;
    NamedPipe& operator =(const NamedPipe& lhs) = delete;
    NamedPipe& operator =(NamedPipe&& rhs)      = delete;
};

//---------------------------------------------------------------------------//

// スレッド間でデータを受け渡すための構造体
struct NamedPipe::DATA
{
    HANDLE evt_close      = nullptr;
    HANDLE evt_disconnect = nullptr;

    DWORD max_instance    = 0;

    union
    {
        DWORD cb_in       = 0;
        DWORD cb_write;
    };
    union
    {
        DWORD cb_out      = 0;
        DWORD cb_read;
    };
    union
    {
        HANDLE pipe_in    = INVALID_HANDLE_VALUE;
        HANDLE pipe_write;
    };
    union
    {
        HANDLE pipe_out   = INVALID_HANDLE_VALUE;
        HANDLE pipe_read;
    };
};

//---------------------------------------------------------------------------//

// パイプデータを読み書きする関数オブジェクトのインターフェイス
interface NamedPipe::IReadWrite
{
    virtual ~IReadWrite() = default;
    virtual void __stdcall ReadWrite(const NamedPipe::DATA& data) = 0;
};

//---------------------------------------------------------------------------//

// NamedPipe.hpp