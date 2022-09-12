//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_TCPCONNECTION_H
#define TOYWEBSERVER_TCPCONNECTION_H

#include <netinet/in.h>

//#include "Channel.h"
//#include "EventLoop.h"
//#include "tools/network.h"
#include "http/HttpService.h"
#include "tools/Buffer.h"


class EventLoop;
class Channel;
class HttpService;
class Server;
class TcpConnection;

class TcpConnectionFactory{
public:
    static shared_ptr<TcpConnection> create(int fd, string name, const sockaddr_in* addr, const shared_ptr<Server>& server,
                                    const shared_ptr<ServiceFactory>& serviceFact
    );
};

class TcpConnection : public enable_shared_from_this<TcpConnection>{
    int fd_;
    string name_;
    struct sockaddr_in addrin;

    std::shared_ptr<Channel> channel;
//    shared_ptr<EventLoop> eventLoop_; // Connection 不需要知道 channel 被哪个 loop 运行

    Buffer buf;
    std::string toWrite;

    shared_ptr<Service> service;
public:
    friend class TcpConnectionFactory;

    bool alive; // todo 备用

    TcpConnection(int fd, string name, const sockaddr_in* addr);

    std::shared_ptr<Channel> getChannel();
    // 传递出去其实不太影响，因为channel本身不拥有资源

    string getName();
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void release();
    ~TcpConnection();
};


#endif //TOYWEBSERVER_TCPCONNECTION_H
