//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_TCPCONNECTION_H
#define TOYWEBSERVER_TCPCONNECTION_H

#include <netinet/in.h>
#include <csignal>
#include "Channel.h"
#include "EventLoop.h"
#include <memory>
#include <sys/epoll.h>
#include "tools/network.h"
#include "Channel.h"
#include "http/HttpService.h"

class EventLoop;
class Channel;
class HttpService;

class TcpConnection : public enable_shared_from_this<TcpConnection>{
    int fd_;
    struct sockaddr_in addrin;
    std::shared_ptr<Channel> channel;

    shared_ptr<EventLoop> eventLoop_;

    vector<char> buf;
    std::string toWrite;

    HttpService service;
public:
    bool alive;

    TcpConnection(int fd, shared_ptr<EventLoop> eventLoop);
    std::shared_ptr<Channel>& getChannel();

    shared_ptr<EventLoop> getLoop();

    void setReadCallback(function<void()> func);
    void setCloseCallback(function<void()> func);
    void handleRead();

    void handleWrite();
    void handleClose();
    void release();
    ~TcpConnection();
};


#endif //TOYWEBSERVER_TCPCONNECTION_H
