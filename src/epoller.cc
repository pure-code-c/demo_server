#include "../include/epoller.h"

namespace demo
{
    Epoller::Epoller(int thread_count_)
        : pool(thread_count_)
    {
        assert(thread_count_ > 0);
        thread_count = thread_count_;
        epoll_fd = epoll_create(thread_count_);
    }

    void Epoller::loop()
    {
        assert(!is_looping);
        is_looping = true;
        pool.start();
        while (is_looping)
        {
            epoll_event events[thread_count];
            int count = epoll_wait(epoll_fd, events, thread_count, -1);
            if (count > 0)
                for (int i = 0; i < count; ++i)
                {
                    Event *event = (Event *)events[i].data.ptr;
                    if (events[i].events & (EPOLLIN | EPOLLRDHUP))
                        pool.addTask(event->read_cb);
                    else if (events[i].events & EPOLLOUT)
                        pool.addTask(event->write_cb);
                }
        }
    }

    void Epoller::addEvent(int fd, Event *event)
    {
        std::lock_guard<std::mutex> lock(mutex);
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event->event);
    }

    void Epoller::modEvent(int fd, Event *event)
    {
        std::lock_guard<std::mutex> lock(mutex);
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event->event);
    }

    void Epoller::delEvent(int fd)
    {
        std::lock_guard<std::mutex> lock(mutex);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    }
}