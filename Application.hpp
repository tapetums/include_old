// Application.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// メッセージループをカプセル化するクラス
// （ゲームループ内包）
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>

//---------------------------------------------------------------------------//

// メッセージループをカプセル化するクラス
class Application
{
public:
    typedef void (__stdcall* GameFunc)(void* arglist);

public:
    Application();
    ~Application();

public:
    int32_t __stdcall Run();
    int32_t __stdcall Run(uint16_t numerator, uint16_t denominator);
    void    __stdcall Exit(int32_t nExitCode = 0);
    void    __stdcall StartGameFunc();
    void    __stdcall StopGameFunc();
    bool    __stdcall AppendGameFunc(GameFunc func, void* arglist = nullptr);
    bool    __stdcall RemoveGameFunc(GameFunc func);

private:
    bool m_is_running     = false;
    bool m_is_game_active = false;

private:
    struct Impl;
    Impl* pimpl;

private:
    Application(const Application&)             = delete;
    Application(Application&&)                  = delete;
    Application& operator= (const Application&) = delete;
    Application& operator= (Application&&)      = delete;
};

//---------------------------------------------------------------------------//

// Application.hpp