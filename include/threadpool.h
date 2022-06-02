#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <assert.h>
#include <stdint.h>
#include "noncopyable.h"

namespace demo
{

    class ThreadPool : Noncopyable
    {
        using Task = std::function<void()>;

    public:
        ThreadPool(uint32_t thread_count_ = 4);
        //开启线程池
        void start();
        //关闭线程池
        void stop();
        //添加任务
        void addTask(const Task &task);

    private:
        //线程运行函数
        void runInThread();

    private:
        std::atomic_bool is_running;       //运行状态
        uint32_t thread_count;             //线程数量
        std::vector<std::thread> threads;  //线程容器
        std::mutex mutex;                  //
        std::condition_variable not_empty; //
        std::list<Task> tasklist;          //任务队列
    };
}