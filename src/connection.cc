#include "../include/connection.h"

namespace demo
{
    std::unordered_map<int, std::shared_ptr<Connection>> Connection::connections;
    Connection::CallBack Connection::read_cb = std::bind(&Connection::defaultReadCb, std::placeholders::_1);
    Connection::CallBack Connection::write_cb = std::bind(&Connection::defaultWrietCb, std::placeholders::_1);
    Connection::CallBack Connection::close_cb = std::bind(&Connection::defaultCloseCb, std::placeholders::_1);

    Connection::Connection(int fd_, sockaddr_in sockad_, Epoller &epoller_)
        : fd(fd_), sockad(sockad_), epoller(epoller_)
    {
        // ET加nonblock
        int flag = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flag | O_NONBLOCK);
        event.event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        event.read_cb = std::bind(read_cb, this);
        event.write_cb = std::bind(write_cb, this);
        epoller.addEvent(fd, &event);
    }

    void Connection::defaultReadCb(Connection *conn)
    {
        conn->buf_mutex.lock();
        int read_len = conn->buf.readFromFd(conn->getFd());
        conn->buf_mutex.unlock();
        //接受数据
        if (read_len > 0)
        {
            conn->event.event.events = EPOLLIN | EPOLLHUP | EPOLLOUT | EPOLLET;
            conn->epoller.modEvent(conn->fd, &conn->event);
        }
        //断开连接
        else
            close_cb(conn);
    }

    void Connection::defaultWrietCb(Connection *conn)
    {

        size_t readable = conn->buf.readableSize();
        char buf[readable];
        for (size_t i = 0; i < readable; ++i)
        {
            conn->buf_mutex.lock();
            buf[i] = toupper((char)conn->buf.readInt8());
            conn->buf_mutex.unlock();
        }
        write(conn->fd, buf, readable);

        conn->event.event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
        conn->epoller.modEvent(conn->fd, &conn->event);
    }

    void Connection::defaultCloseCb(Connection *conn)
    {
        conn->epoller.delEvent(conn->getFd());
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &conn->getSockad().sin_addr, buf, INET_ADDRSTRLEN);
        printf("client %s:%d disconnect\n", buf, ntohs(conn->getSockad().sin_port));
        Connection::connections.erase(conn->getFd());
        close(conn->getFd());
    }
}