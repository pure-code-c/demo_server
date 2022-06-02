#include "../include/listener.h"

namespace demo
{
    Listener::Listener(int port_, Epoller &epoller_)
        : port(port_), epoller(epoller_)
    {
        //初始化监听
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        int on = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &on, sizeof(on));
        sockad.sin_family = AF_INET;
        sockad.sin_port = htons(port);
        sockad.sin_addr.s_addr = htonl(INADDR_ANY);
        int ret = bind(listen_fd, (sockaddr *)&sockad, sizeof(sockad));
        assert(ret == 0);
        listen(listen_fd, 10);
        //注册事件
        event.event.events = EPOLLIN | EPOLLET;
        event.read_cb = std::bind(&Listener::listen_cb, this);
        epoller.addEvent(listen_fd, &event);
    }

    void Listener::listen_cb()
    {
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock = accept(listen_fd, (sockaddr *)&client_addr, &addr_len);

        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN);

        printf("client %s:%d connnect\n", buf, ntohs(client_addr.sin_port));

        Connection *connection = new Connection(client_sock, client_addr, epoller);
        Connection::connections[client_sock].reset(connection);
    }
}