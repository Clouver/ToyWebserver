//
// Created by Clouver on 2022/8/14.
//

#ifndef TOYWEBSERVER_SERVER_H
#define TOYWEBSERVER_SERVER_H

#include "EventLoop.h"
#include <sys/socket.h>
#include <functional>
#include <fcntl.h>
#include <csignal>
#include <unistd.h>
#include "tools/network.h"
#include "TcpConnection.h"
#include "EventLoop.h"


class Server {
    shared_ptr<EventLoop> mainLoop;
    vector<shared_ptr<EventLoop>>subLoops;
    vector<shared_ptr<std::thread>> threads; // todo 肯定要封装成线程池 尝试无锁队列

    int fd_;
    int port_;
    int threadNum_;
    int maxConnSize_;

    shared_ptr<Channel>ch_;

    // todo 设计上不该存在，权当条件变量的使用练习了。connection 在 subreactor 内的部分应该交给它自己处；
    //  在Server里只有 connOfFd， unordered_map 感觉上不同线程read/write/erase互斥的key是线程安全的？
    // 关闭连接
    shared_ptr<std::thread>connectionManager;
    mutex connMutex;
    bool connManaging{};
    condition_variable connCond;
    queue<int>toClose;
    unordered_map<int, shared_ptr<TcpConnection>>connOfFd;

public:
    Server();

    Server(int port, int threadNum, int maxConnSize);

    ~Server();

    shared_ptr<EventLoop> getLoop() const ;

    void runSubreactors();

    void setMainLoop();

    void runConnManager();

    void start();

    void handleDestroyingConn();

    void handleClose(int fd);

    void handleNewConn();
};


#endif //TOYWEBSERVER_SERVER_H
