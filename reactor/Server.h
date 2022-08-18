//
// Created by Clouver on 2022/8/14.
//

#ifndef TOYWEBSERVER_SERVER_H
#define TOYWEBSERVER_SERVER_H

#include "EventLoop.h"
#include "ThreadPool/ThreadPool.h"
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
    vector<shared_ptr<std::thread>> threads;

    int fd_;
    int port_;
    int threadNum_;
    int maxConnSize_;

    shared_ptr<Channel>ch_;

    // 关闭连接
    shared_ptr<std::thread>connectionManager;
    mutex connMutex;
    bool connManaging{};
    condition_variable connCond;
    queue<int>toClose;
    unordered_map<int, shared_ptr<TcpConnection>>connOfFd;

public:
    Server();
    shared_ptr<EventLoop> getLoop() const ;

    void runSubreactors();

    void setMainLoop();

    void destroyClosedConn();

    void runConnManager();

    Server(int port, int threadNum, int maxConnSize);

    void handleClose(int fd);

    void handleConn();

    void start();

    ~Server();
};


#endif //TOYWEBSERVER_SERVER_H
