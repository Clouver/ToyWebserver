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

    // 关闭连接
    shared_ptr<std::thread>connectionManager;
    mutex connMutex;
    bool connManaging;
    condition_variable connCond;
    queue<int>toClose;
    unordered_map<int, shared_ptr<TcpConnection>>connOfFd;


public:
    Server():Server(0,0,0){};

    shared_ptr<EventLoop> getLoop() const {
        return mainLoop;
    }

    void runSubreactors(){
        for(int i=0; i<threadNum_; i++){
            subLoops.emplace_back(new EventLoop());
            subLoops.back()->start(); // 必须先设置 start，再创建线程。
            threads.push_back( make_shared<std::thread>(bind(&EventLoop::loop, subLoops.back())) );
//            subLoops.back()->bindThread(threads.back());
        }
    }

    void setMainLoop(){
        fd_ = createBindListen(port_);
        if(fd_ == -1)
            throw exception();

        /*
         * AcceptChannel 的定义。
         * 传入server指针。
         * 放入mainloop。
         *
         * acceptchannel 的 owner 为空
         */
        mainLoop = make_shared<EventLoop>();
        shared_ptr<TcpConnection>fakeConn = make_shared<TcpConnection>(fd_, mainLoop);
        fakeConn->getChannel()->setOwner(fakeConn);
        fakeConn->setReadCallback( bind(&Server::handleConn, this));
        connOfFd[fd_] = fakeConn;
    }

    void tmp(){
        while(connManaging){
            unique_lock<mutex> lock(connMutex);
            while(toClose.empty())
                connCond.wait(lock);

            while(!toClose.empty()){
                int fd = toClose.front();
                toClose.pop();
                shared_ptr<TcpConnection> conn = connOfFd[fd];
                connOfFd.erase(fd); // 析构即释放资源
                conn->release();
            }
            lock.unlock();
        }
    }

    void runConnManager(){
        connManaging = true;
//        function =
        connectionManager = make_shared<std::thread>( bind(&Server::tmp, this));
    }

    Server(int port, int threadNum, int maxConnSize):fd_(0), port_(port),
    threadNum_(threadNum),maxConnSize_(maxConnSize){
        /*
         * Server 由几个部分组成：
         *      mainloop                即主reactor。这里不单开线程了，直接主线程进行loop。 也没有单独的 Acceptor，mainloop就是acceptor，只修改一下handleread。
         *      subLoops + threads      即从reactors。线程池。
         *      connOfFd                保存已建立的连接。（包含了acceptChannel所在的伪连接）
         *      connManager             负责释放已关闭的连接。
         */

        setMainLoop();
        runSubreactors();
        runConnManager();
    }

    void handleClose(int fd){
        unique_lock<mutex>lock(connMutex);
        toClose.push(fd);
        lock.unlock();
        connCond.notify_one();
    }

    void handleConn(){

        sockaddr addr{};
        socklen_t len = sizeof(addr);
        int newFd = accept(fd_, &addr, &len);
        setSocketNonBlocking(newFd);

        if(newFd == -1)
            return;

        shared_ptr<EventLoop>&loop = subLoops[rand()%threadNum_];

        shared_ptr<TcpConnection> conn = make_shared<TcpConnection>(newFd, loop);
        conn->getChannel()->setOwner(conn);
        conn->setCloseCallback( bind(&Server::handleClose, this, newFd));
        connOfFd[newFd] = conn;
    }


    void start(){


        mainLoop->start();
        mainLoop->loop();
    }

    ~Server(){
        for(int i=0; i<threadNum_; i++)
            subLoops[i]->stop();
    }
};


#endif //TOYWEBSERVER_SERVER_H
