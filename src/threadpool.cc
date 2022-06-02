#include "../include/threadpool.h"

namespace demo
{

    ThreadPool::ThreadPool(uint32_t thread_count_)
    {
        assert(thread_count_ > 0);
        is_running = false;
        thread_count = thread_count_;
        threads.reserve(thread_count);
    }

    void ThreadPool::start()
    {
        assert(!is_running);
        is_running = true;
        for (int i = 0; i < thread_count; ++i)
            threads.push_back(std::thread(&ThreadPool::runInThread, this));
    }

    void ThreadPool::stop()
    {

        while (!tasklist.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        is_running = false;
        not_empty.notify_all();

        for (std::thread &t : threads)
        {
            if (t.joinable())
                t.join();
        }
    }

    void ThreadPool::addTask(const Task &task)
    {
        assert(is_running);
        std::unique_lock<std::mutex> mu(mutex);
        tasklist.push_back(task);
        not_empty.notify_one();
    }

    void ThreadPool::runInThread()
    {
        while (is_running)
        {
            Task task;
            {
                std::unique_lock<std::mutex> mu(mutex);
                if (!tasklist.empty())
                {
                    task = tasklist.front();
                    tasklist.pop_front();
                }
                else if (is_running && tasklist.empty())
                    not_empty.wait(mu);
            }
            if (task)
                task();
        }
    }
}