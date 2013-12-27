// Application.cpp

//---------------------------------------------------------------------------//
//
// メッセージループをカプセル化するクラス
// （ゲームループ内包）
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <list>

#include <windows.h>

#include "DebugPrint.hpp"

#include "Application.hpp"

//---------------------------------------------------------------------------//

struct Timer
{
    static const size_t prevention_point = 1 << (32 - 1);

    int64_t  frequency;
    int64_t  start_time;
    int64_t  next_time;
    uint16_t fps_numerator;
    uint16_t fps_denominator;
    size_t   frame_count;

    Timer()
    {
        this->Reset();
    }

    ~Timer()
    {
    }

    void Reset()
    {
        ::QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
        ::QueryPerformanceCounter((LARGE_INTEGER*)&start_time);

        next_time   = 0;
        frame_count = 0;

        console_out(TEXT("PerformanceFrequency: %I64d"), frequency);
    }

    void SetFPS(uint16_t numerator, uint16_t denominator)
    {
        fps_numerator   = numerator;
        fps_denominator = denominator;

        console_out
        (
            TEXT("Target FPS: %Lf"),
            (denominator != 0) ?
            numerator * 1000.0L / denominator :
            0.0L
        );
        console_out
        (
            TEXT("Time Interval: %Lfms"),
            (numerator != 0) ?
            1.0L * denominator / numerator :
            0.0L
        );
    }

    bool HasTimeCome() const
    {
        // FPS 設定値のどちらかが 0 の場合、更新時刻は永遠にやって来ない
        if ( 0 == (fps_numerator & fps_denominator) )
        {
            return false;
        }

        int64_t present_time;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&present_time);
    /// console_out(TEXT("present_time: %I64d"), present_time);

        return (present_time >= next_time);
    }

    void SetNextTime()
    {
        // FPS 設定値のどちらかが 0 なら何もしない
        if ( 0 == (fps_numerator & fps_denominator) )
        {
            return;
        }

        // 念のためオーバーフロー防止
        if ( frame_count > prevention_point )
        {
            // 32 ビット環境を最悪として想定。
            // 2,147,483,648 フレームを超えたらカウンタをリセットします。
            // 連続稼働させた場合 60fps の場合で 約 414.25 日後、
            // 240fps の場合で 約 103.5 日後。
            console_out(TEXT("Reset frame count to avoid integer overflow"));
            this->Reset();
        }
    /// console_out(TEXT("frame_count: %u"), frame_count);

        int64_t present_time;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&present_time);

        // 処理が追いつかなくて何フレームか飛ばされた場合も考慮して、ループ
        while ( next_time < present_time )
        {
            // 小数点以下切り捨てによる誤差蓄積を回避するため、
            // 常に開始時を起点として計算。
            ++frame_count;
            next_time = start_time +
                        (frame_count * frequency * fps_denominator) /
                        (fps_numerator * 1000);
        }
    /// console_out(TEXT("next_time: %I64d, present_time: %I64d"), next_time, present_time);
    }
};

//---------------------------------------------------------------------------//

struct Application::Impl
{
    struct Item { GameFunc execute; void* arglist; };
    std::list<Item> func_list;
};

//---------------------------------------------------------------------------//

Application& __stdcall Application::app()
{
    static Application app_;

    return app_;
}

//---------------------------------------------------------------------------//

Application::Application()
{
    pimpl = new Impl;
}

//---------------------------------------------------------------------------//

Application::~Application()
{
    this->Exit(0);

    delete pimpl;
    pimpl = nullptr;
}

//---------------------------------------------------------------------------//

int32_t __stdcall Application::Run()
{
    MSG msg = {};

    console_out(TEXT("---------------- Message Loop ----------------"));

    m_is_running = true;
    while ( m_is_running )
    {
        if ( ::GetMessage(&msg, nullptr, 0, 0) < 1 )
        {
            break;
        }
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    console_out(TEXT("---------------- Message Loop ----------------"));

    return static_cast<int32_t>(msg.wParam);
}

//---------------------------------------------------------------------------//

int32_t __stdcall Application::Run(uint16_t numerator, uint16_t denominator)
{
    MSG msg = {};

    Timer timer;
    timer.SetFPS(numerator, denominator);
    timer.SetNextTime();

    console_out(TEXT("---------------- Message Loop ----------------"));

    const auto func_list = &pimpl->func_list;

    m_is_running = true;
    while ( m_is_running )
    {
        if ( ::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE) )
        {
            if ( ::GetMessage(&msg, nullptr, 0, 0) < 1 )
            {
                break;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else if ( m_is_game_active )
        {
            if ( timer.HasTimeCome() )
            {
                for ( const auto& item: *func_list )
                {
                    item.execute(item.arglist);
                }
                timer.SetNextTime();
            }
        }
        else
        {
            ::MsgWaitForMultipleObjects
            (
                0, nullptr, FALSE, INFINITE, QS_ALLINPUT
            );
        }
    }

    console_out(TEXT("---------------- Message Loop ----------------"));

    return static_cast<int32_t>(msg.wParam);
}

//---------------------------------------------------------------------------//

void __stdcall Application::Exit(int32_t nExitCode)
{
    console_out(TEXT("Application::Exit() begin"));

    m_is_running = false;

    ::PostQuitMessage(nExitCode);

    console_out(TEXT("Application::Exit() end"));
}

//---------------------------------------------------------------------------//

void __stdcall Application::StartGameFunc()
{
    console_out(TEXT("Application::StartGameFunc() begin"));

    m_is_game_active = true;

    console_out(TEXT("Application::StartGameFunc() end"));
}

//---------------------------------------------------------------------------//

void __stdcall Application::StopGameFunc()
{
    console_out(TEXT("Application::StopGameFunc() begin"));

    m_is_game_active = false;

    console_out(TEXT("Application::StopGameFunc() end"));
}

//---------------------------------------------------------------------------//

bool __stdcall Application::AppendGameFunc(GameFunc func, void* arglist)
{
    console_out(TEXT("Application::AppendGameFunc() begin"));

    if ( nullptr == func )
    {
        console_out(TEXT("Invalid function pointer"));
        console_out(TEXT("Application::AppendGameFunc() end"));
        return false;
    }

    Application::Impl::Item item = { func, arglist };
    pimpl->func_list.push_back(std::move(item));

    console_out(TEXT("Application::AppendGameFunc() end"));

    return true;
}

//---------------------------------------------------------------------------//

bool __stdcall Application::RemoveGameFunc(GameFunc func)
{
    console_out(TEXT("Application::RemoveGameFunc() begin"));

    if ( nullptr == func )
    {
        console_out(TEXT("Invalid function pointer"));
        console_out(TEXT("Application::RemoveGameFunc() end"));
        return false;
    }

    auto it = pimpl->func_list.rbegin();
    const auto end = pimpl->func_list.rend();
    while ( it != end )
    {
        if ( (*it).execute == func )
        {
            pimpl->func_list.erase(it.base());
            console_out(TEXT("Application::RemoveGameFunc() end"));
            return true;
        }
        ++it;
    }

    console_out(TEXT("Could not find the game function"));
    console_out(TEXT("Application::RemoveGameFunc() end"));

    return false;
}

//---------------------------------------------------------------------------//

// Application.cpp