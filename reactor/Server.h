//
// Created by Clouver on 2022/8/14.
//

#ifndef TOYWEBSERVER_SERVER_H
#define TOYWEBSERVER_SERVER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <memory>
#include <map>

//#include "Service.h"


class EventLoop;
class Channel;
class TcpConnectionFactory;
class TcpConnection;
class ServiceFactory;

using namespace std;

class Server : public std::enable_shared_from_this<Server>{
    shared_ptr<EventLoop> mainLoop;
    vector<shared_ptr<EventLoop>>subLoops;
    vector<shared_ptr<thread>> threads; // todo 肯定要封装成线程池 尝试无锁队列

    int fd_;
    int port_;
    int threadNum_;
    int maxConnSize_;

    shared_ptr<ServiceFactory> connFact;

    shared_ptr<Channel>ch_;

    map<string, shared_ptr<TcpConnection>>connOfFd;
    int connId;
    mutex connMutex;    // todo 不如改成 hashmap 避免竞争？

public:
    Server();

    Server(int port, int threadNum, int maxConnSize, shared_ptr<ServiceFactory> serviceFact);

    ~Server();

    shared_ptr<EventLoop> getLoop() const ;

    void runSubreactors();

    void setMainLoop();

    void start();

    // todo 存在conn已销毁的可能。 只用于channel回调的情况下安全。
    void handleClose(TcpConnection *pconn);

    void handleNewConn();
};


#endif //TOYWEBSERVER_SERVER_H
