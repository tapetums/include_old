// Task.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// タスク処理用スレッドプールクラス
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

#include <functional>

//---------------------------------------------------------------------------//

class TaskWorker;
class TaskManager;

typedef std::function<void (TaskWorker*)> Task;

//---------------------------------------------------------------------------//

class TaskWorker
{
public:
    explicit TaskWorker(TaskManager* manager);
    TaskWorker(TaskWorker&& rhs);
    ~TaskWorker();

public:
    bool         __stdcall empty()      const;
    bool         __stdcall is_running() const;
    size_t       __stdcall task_count() const;
    TaskManager* __stdcall manager()    const;

public:
    void __stdcall AddTask(const Task& task);
    Task __stdcall QueryTask();
    Task __stdcall StealTask();
    bool __stdcall Start();
    bool __stdcall Stop();
    void __stdcall Pause();
    void __stdcall Resume();
    bool __stdcall WaitEmpty(DWORD dwMilliseconds = INFINITE);

private:
    TaskWorker(const TaskWorker&)             = delete;
    TaskWorker& operator= (const TaskWorker&) = delete;
    TaskWorker& operator= (TaskWorker&&)      = delete;

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

class TaskManager
{
public:
    explicit TaskManager(size_t worker_count = 0);
    ~TaskManager();

public:
    bool        __stdcall empty()              const;
    bool        __stdcall is_running()         const;
    size_t      __stdcall worker_count()       const;
    TaskWorker* __stdcall worker(size_t index) const;

public:
    void __stdcall AddTask(const Task& task);
    Task __stdcall QueryTask();
    bool __stdcall Start();
    bool __stdcall Stop();
    void __stdcall Pause();
    void __stdcall Resume();
    bool __stdcall WaitEmpty(DWORD dwMilliseconds = INFINITE);

private:
    TaskManager(const TaskManager&)             = delete;
    TaskManager(TaskManager&&)                  = delete;
    TaskManager& operator= (const TaskManager&) = delete;
    TaskManager& operator= (TaskManager&&)      = delete;

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

// Task.hpp