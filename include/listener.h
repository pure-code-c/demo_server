#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "epoller.h"
#include "noncopyable.h"
#include "connection.h"

namespace demo
{

    class Listener : Noncopyable
    {
    public:
        Listener(int port_, Epoller &epoller_);

    private:
        void listen_cb();

    private:
        int listen_fd;
        int port;
        sockaddr_in sockad;
        Event event;
        Epoller &epoller;
    };
}