// Task.cpp

//---------------------------------------------------------------------------//
//
// タスク処理用スレッドプールクラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <vector>
#include <deque>
#include <functional>
#include <thread>

#include <windows.h>

#include "DebugPrint.hpp"
#include "Task.hpp"

//---------------------------------------------------------------------------//

static const Task null_task;

//---------------------------------------------------------------------------//
//
// class TaskWorker
//
//---------------------------------------------------------------------------//

struct TaskWorker::Impl
{
    CRITICAL_SECTION cs;

    TaskManager* manager;
    bool         on_work;
    size_t       task_count;
    DWORD        thread_id;
    HANDLE       evt_empty;

    std::deque<Task> deque;
    std::thread      thread;
};

//---------------------------------------------------------------------------//

TaskWorker::TaskWorker(TaskManager* manager)
{
    console_out(TEXT("TaskWorker::ctor() begin"));

    pimpl = new Impl;

    ::InitializeCriticalSection(&pimpl->cs);

    pimpl->manager    = manager;
    pimpl->on_work    = false;
    pimpl->task_count = 0;
    pimpl->thread_id  = 0;
    pimpl->evt_empty  = ::CreateEvent(nullptr, TRUE, TRUE, nullptr);

    console_out(TEXT("TaskWorker::ctor() end"));
}

//---------------------------------------------------------------------------//

TaskWorker::TaskWorker(TaskWorker&& rhs)
{
    console_out(TEXT("TaskWorker::ctor(move) begin"));

    pimpl = rhs.pimpl;

    rhs.pimpl = nullptr;

    console_out(TEXT("TaskWorker::ctor(move) end"));
}

//---------------------------------------------------------------------------//

TaskWorker::~TaskWorker()
{
    console_out(TEXT("TaskWorker::dtor() begin"));

    if ( nullptr == pimpl )
    {
        console_out(TEXT("Already moved"));
        console_out(TEXT("TaskWorker::dtor() end"));
        return;
    }

    this->Stop();

    ::CloseHandle(pimpl->evt_empty);
    pimpl->evt_empty = nullptr;

    ::DeleteCriticalSection(&pimpl->cs);

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("TaskWorker::dtor() end"));
}

//---------------------------------------------------------------------------//

bool __stdcall TaskWorker::empty() const
{
    return (pimpl->task_count == 0);
}

//---------------------------------------------------------------------------//

bool __stdcall TaskWorker::is_running() const
{
    return (pimpl->thread_id != 0);
}

//---------------------------------------------------------------------------//

size_t __stdcall TaskWorker::task_count() const
{
    return pimpl->task_count;
}

//---------------------------------------------------------------------------//

TaskManager* __stdcall TaskWorker::manager() const
{
    return pimpl->manager;
}

//---------------------------------------------------------------------------//

void __stdcall TaskWorker::AddTask(const Task& task)
{
    //console_out(TEXT("TaskWorker::AddTask() begin"));

    ::InterlockedIncrement(&pimpl->task_count);

    ::EnterCriticalSection(&pimpl->cs);

    pimpl->deque.push_back(task);

    ::LeaveCriticalSection(&pimpl->cs);

    ::ResetEvent(pimpl->evt_empty);

    //console_out(TEXT("TaskWorker::AddTask() end"));
}

//---------------------------------------------------------------------------//

Task __stdcall TaskWorker::QueryTask()
{
    //console_out(TEXT("TaskWorker::QueryTask() begin"));

    ::EnterCriticalSection(&pimpl->cs);

    if ( pimpl->deque.empty() )
    {
        ::LeaveCriticalSection(&pimpl->cs);
        //console_out(TEXT("TaskWorker::QueryTask() end"));
        return std::move(null_task);
    }

    auto task = pimpl->deque.front();
    pimpl->deque.pop_front();

    ::LeaveCriticalSection(&pimpl->cs);

    ::InterlockedDecrement(&pimpl->task_count);

    //console_out(TEXT("TaskWorker::QueryTask() end"));

    return std::move(task);
}

//---------------------------------------------------------------------------//

Task __stdcall TaskWorker::StealTask()
{
    //console_out(TEXT("TaskWorker::StealTask() begin"));

    ::EnterCriticalSection(&pimpl->cs);

    if ( pimpl->deque.empty() )
    {
        ::LeaveCriticalSection(&pimpl->cs);
        //console_out(TEXT("TaskWorker::StealTask() end"));
        return std::move(null_task);
    }

    auto task = pimpl->deque.front();
    pimpl->deque.pop_back();

    ::LeaveCriticalSection(&pimpl->cs);

    ::InterlockedDecrement(&pimpl->task_count);

    console_out(TEXT("Stole other worker's task"));

    //console_out(TEXT("TaskWorker::StealTask() end"));

    return std::move(task);
}

//---------------------------------------------------------------------------//

bool __stdcall TaskWorker::Start()
{
    console_out(TEXT("TaskWorker::Start() begin"));

    if ( this->is_running() )
    {
        console_out(TEXT("Already started"));
        this->Resume();
        console_out(TEXT("TaskWorker::Start() end"));
        return false;
    }

    // スレッドの開始
    pimpl->thread = std::thread([=]() -> void
    {
        pimpl->thread_id = ::GetCurrentThreadId();
        console_out(TEXT("Started @ 0x%08u"), pimpl->thread_id);

        MSG msg = { };

        pimpl->on_work = true;

        // COM の初期化
        ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        // メインループ
        const auto manager = pimpl->manager;
        while ( true )
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
            else if ( pimpl->on_work )
            {
                auto task = this->QueryTask();
                if ( static_cast<bool>(task) )
                {
                    ::InterlockedIncrement(&pimpl->task_count);
                    {
                        task(this);
                    }
                    ::InterlockedDecrement(&pimpl->task_count);
                    continue;
                }

                auto others_task = manager->QueryTask();
                if ( static_cast<bool>(others_task) )
                {
                    ::InterlockedIncrement(&pimpl->task_count);
                    {
                        others_task(this);
                    }
                    ::InterlockedDecrement(&pimpl->task_count);
                    continue;
                }

                console_out(TEXT("empty"));
                pimpl->on_work = false;
                ::SetEvent(pimpl->evt_empty);
            }
            else
            {
                ::MsgWaitForMultipleObjects
                (
                    0, nullptr, FALSE, INFINITE, QS_ALLINPUT
                );
            }
        }

        // COM の終了処理
        ::CoUninitialize();
 
        console_out(TEXT("Stopped @ 0x%08u"), pimpl->thread_id);
        pimpl->thread_id = 0;
    });
    
    // スレッドが開始されるまで待つ
    while ( pimpl->thread_id == 0 )
    {
        ::Sleep(1);
    }

    console_out(TEXT("TaskWorker::Start() end"));
    return true;
}

//---------------------------------------------------------------------------//

bool __stdcall TaskWorker::Stop()
{
    console_out(TEXT("TaskWorker::Stop() begin"));

    if ( ! this->is_running() )
    {
        console_out(TEXT("Already stopped"));
        console_out(TEXT("TaskWorker::Stop() end"));
        return false;
    }

    // スレッドに終了を通知
    ::PostThreadMessage(pimpl->thread_id, WM_QUIT, 0, 0);

    // スレッドの完了を待つ
    console_out(TEXT("Waiting for the thread to stop..."));
    if ( pimpl->thread.joinable() )
    {
        pimpl->thread.join();
    }
    console_out(TEXT("Thread stopped"));

    console_out(TEXT("TaskWorker::Stop() end"));
    return true;
}

//---------------------------------------------------------------------------//

void __stdcall TaskWorker::Pause()
{
    console_out(TEXT("Paused"));

    pimpl->on_work = false;

    console_out(TEXT("Resumed"));
}

//---------------------------------------------------------------------------//

void __stdcall TaskWorker::Resume()
{
    console_out(TEXT("TaskWorker::Resume() begin"));

    pimpl->on_work = true;
    ::PostThreadMessage(pimpl->thread_id, WM_NULL, 0, 0);

    console_out(TEXT("TaskWorker::Resume() end"));
}

//---------------------------------------------------------------------------//

bool __stdcall TaskWorker::WaitEmpty(DWORD dwMilliseconds)
{
    console_out(TEXT("TaskWorker::WaitEmpty() begin"));

    if ( pimpl->task_count == 0 )
    {
        console_out(TEXT("Already empty"));
        console_out(TEXT("TaskWorker::WaitEmpty() end"));
        return true;
    }

    auto result = ::WaitForSingleObject(pimpl->evt_empty, dwMilliseconds);

    console_out(TEXT("TaskWorker::WaitEmpty() end"));

    return (result == WAIT_OBJECT_0);
}

//---------------------------------------------------------------------------//
//
// class TaskManager
//
//---------------------------------------------------------------------------//

struct TaskManager::Impl
{
    CRITICAL_SECTION cs;
    bool is_running = false;
    std::vector<TaskWorker> workers;
    std::vector<TaskWorker>::iterator it;

    void __stdcall IncIterator();
};

//---------------------------------------------------------------------------//

void __stdcall TaskManager::Impl::IncIterator()
{
    ::EnterCriticalSection(&cs);
    {
        ++it;
        if ( it == workers.end() )
        {
            it = workers.begin();
        }
    }
    ::LeaveCriticalSection(&cs);
}

//---------------------------------------------------------------------------//

TaskManager::TaskManager(size_t worker_count)
{
    console_out(TEXT("TaskManager::ctor() begin"));

    pimpl = new Impl;

    ::InitializeCriticalSection(&pimpl->cs);

    SYSTEM_INFO si = { };
    ::GetNativeSystemInfo(&si);

    size_t count;
    if ( 0 < worker_count && worker_count <= si.dwNumberOfProcessors )
    {
        count = worker_count;
    }
    else
    {
        count = si.dwNumberOfProcessors;
    }
    while ( count > 0 )
    {
        pimpl->workers.emplace_back(this);
        --count;
    }

    pimpl->it = pimpl->workers.begin();

    console_out(TEXT("TaskManager::ctor() end"));
}

//---------------------------------------------------------------------------//

TaskManager::~TaskManager()
{
    console_out(TEXT("TaskManager::dtor() begin"));

    this->Stop();

    ::DeleteCriticalSection(&pimpl->cs);

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("TaskManager::dtor() end"));
}

//---------------------------------------------------------------------------//

bool __stdcall TaskManager::empty() const
{
    for ( auto& worker: pimpl->workers )
    {
        if ( ! worker.empty() )
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------//

bool __stdcall TaskManager::is_running() const
{
    return pimpl->is_running;
}

//---------------------------------------------------------------------------//

size_t __stdcall TaskManager::worker_count() const
{
    return pimpl->workers.size();
}

//---------------------------------------------------------------------------//

TaskWorker* __stdcall TaskManager::worker(size_t index) const
{
    TaskWorker* worker;

    try
    {
        worker = &pimpl->workers.at(index);
    }
    catch ( std::out_of_range& )
    {
        console_out(TEXT("std::out_of_range: worker(%u) in %u"), index, pimpl->workers.size());
        worker = nullptr;
    }

    return worker;
}

//---------------------------------------------------------------------------//

void __stdcall TaskManager::AddTask(const Task& task)
{
    ::EnterCriticalSection(&pimpl->cs);
    {
        (*(pimpl->it)).AddTask(task);
    }
    ::LeaveCriticalSection(&pimpl->cs);

    pimpl->IncIterator();
}

//---------------------------------------------------------------------------//

Task __stdcall TaskManager::QueryTask()
{
    const auto count = pimpl->workers.size();

    for ( size_t n = 0; n < count; ++n )
    {
        ::EnterCriticalSection(&pimpl->cs);

        auto task = (*(pimpl->it)).StealTask();

        ::LeaveCriticalSection(&pimpl->cs);

        if ( static_cast<bool>(task) )
        {
            return std::move(task);
        }

        pimpl->IncIterator();
    }

    return std::move(null_task);
}

//---------------------------------------------------------------------------//

bool __stdcall TaskManager::Start()
{
    console_out(TEXT("TaskManager::Start() begin"));

    if ( pimpl->is_running )
    {
        console_out(TEXT("Already started"));
        console_out(TEXT("TaskManager::Start() end"));
        return false;
    }

    for ( auto& worker: pimpl->workers )
    {
        worker.Start();
    }
    pimpl->is_running = true;

    console_out(TEXT("TaskManager::Start() end"));
    return true;
}

//---------------------------------------------------------------------------//

bool __stdcall TaskManager::Stop()
{
    console_out(TEXT("TaskManager::Stop() begin"));

    if ( ! pimpl->is_running )
    {
        console_out(TEXT("Already stopped"));
        console_out(TEXT("TaskManager::Stop() end"));
        return false;
    }

    for ( auto& worker: pimpl->workers )
    {
        worker.Stop();
    }
    pimpl->is_running = false;

    console_out(TEXT("TaskManager::Stop() end"));
    return true;
}

//---------------------------------------------------------------------------//

void __stdcall TaskManager::Pause()
{
    console_out(TEXT("TaskManager::Pause() begin"));

    for ( auto& worker: pimpl->workers )
    {
        worker.Pause();
    }

    console_out(TEXT("TaskManager::Pause() end"));
}

//---------------------------------------------------------------------------//

void __stdcall TaskManager::Resume()
{
    console_out(TEXT("TaskManager::Resume() begin"));

    for ( auto& worker: pimpl->workers )
    {
        worker.Resume();
    }

    console_out(TEXT("TaskManager::Resume() end"));
}

//---------------------------------------------------------------------------//

bool __stdcall TaskManager::WaitEmpty(DWORD dwMilliseconds)
{
    console_out(TEXT("TaskManager::WaitEmpty() begin"));

    if ( ! pimpl->is_running )
    {
        console_out(TEXT("Not running"));
        console_out(TEXT("TaskManager::WaitEmpty() end"));
        return false;
    }

    for ( auto& worker: pimpl->workers )
    {
        const auto result = worker.WaitEmpty(dwMilliseconds);
        if ( ! result )
        {
            console_out(TEXT("WaitEmpty(): WAIT_TIMEOUT"));
            console_out(TEXT("TaskManager::WaitEmpty() end"));
            return false;
        }
    }

    console_out(TEXT("TaskManager::WaitEmpty() end"));

    return true;
}

//---------------------------------------------------------------------------//

// Task.cpp