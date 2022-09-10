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

    map<int, shared_ptr<TcpConnection>>connOfFd;

public:
    Server();

    Server(int port, int threadNum, int maxConnSize, shared_ptr<ServiceFactory> serviceFact);

    ~Server();

    shared_ptr<EventLoop> getLoop() const ;

    void runSubreactors();

    void setMainLoop();

    void start();

    // 传入一个 connection 的 share，延长 conn 声明周期直到channel reset。
    // 避免 channel 调用回调的时候 connection 没了导致channel自己没了。
    // 坏处是，如果不记得把 channel reset，会和 connection 循环引用。 而且必须在所有操作完成之后再 reset。
    void handleClose(shared_ptr<TcpConnection> pconn);

    void handleNewConn();
};


#endif //TOYWEBSERVER_SERVER_H
