#pragma once

#include <functional>
#include <sys/epoll.h>
#include <mutex>
#include <atomic>

#include "threadpool.h"
#include "noncopyable.h"

namespace demo
{

    struct Event
    {
        using CallBack = std::function<void()>;

        Event() { event.data.ptr = this; }
        int fd;
        epoll_event event;
        CallBack read_cb;
        CallBack write_cb;
    };

    class Epoller : Noncopyable
    {
    public:
        Epoller(int thread_count_ = 10);
        //开始事件循环
        void loop();
        //事件管理
        void addEvent(int fd, Event *event);
        void modEvent(int fd, Event *event);
        void delEvent(int fd);

    private:
        std::atomic_bool is_looping = false;
        int epoll_fd;
        int thread_count;
        ThreadPool pool;
        std::mutex mutex;
    };
}