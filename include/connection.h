#pragma once

#include "epoller.h"
#include "noncopyable.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include "buffer.h"

namespace demo
{

    class Connection : Noncopyable
    {
        using CallBack = std::function<void(Connection *)>;

    public:
        Connection(int fd_, sockaddr_in sockad_, Epoller &epoller_);
        //设置读写断开连接回调
        static void setReadCb(const CallBack &read_cb_) { read_cb = std::move(read_cb_); }
        static void setWriteCb(const CallBack &write_cb_) { write_cb = std::move(write_cb_); }
        static void setCloseCb(const CallBack &close_cb_) { close_cb = std::move(close_cb_); }
        //关闭连接
        void closeConnect();

        //默认读写断开连接回调
        static void defaultReadCb(Connection *conn);
        static void defaultWrietCb(Connection *conn);
        static void defaultCloseCb(Connection *conn);

    public:
        int getFd() { return fd; }
        sockaddr_in &getSockad() { return sockad; }

    private:
        int fd;
        sockaddr_in sockad;
        static CallBack read_cb;
        static CallBack write_cb;
        static CallBack close_cb;

    public:
        Event event;
        Epoller &epoller;
        Buffer buf;
        std::mutex buf_mutex;
        static std::unordered_map<int, std::shared_ptr<Connection>> connections;
    };
}