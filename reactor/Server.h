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

#include "tools/network.h"
#include "TcpConnection.h"
#include "EventLoop.h"

class Server {
    EventLoop mainLoop;

    vector<shared_ptr<EventLoop>>subLoops;
    vector<shared_ptr<std::thread>> threads;

    int fd_;
    int port_;
    int threadNum_;
    int maxConnSize_;

    unordered_map<int, shared_ptr<TcpConnection>>connOfFd;

//    shared_ptr<TcpConnection> conns;

public:
    Server():Server(0,0,0){};

    Server(int port, int threadNum, int maxConnSize):port_(port),
    threadNum_(threadNum),maxConnSize_(maxConnSize){

        for(int i=0; i<threadNum; i++){
            subLoops.emplace_back(new EventLoop());
            subLoops.back()->start();
            threads.push_back( make_shared<std::thread>(bind(&EventLoop::loop, subLoops.back())) );
//            subLoops.back()->bindThread(threads.back());
        }

        fd_ = createBindListen(port);
        if(fd_ == -1)
            throw exception();

        /*
         * AcceptChannel 的定义。
         * 传入server指针。
         * 放入mainloop。
         *
         * acceptchannel 的 owner 为空
         */
        SP_Channel acceptChannel = make_shared<Channel>(fd_);
        acceptChannel->setConnCallback( bind(&Server::handleConn, this));
        mainLoop.addChannel(std::move(acceptChannel) );
    }

    void handleConn(){

        sockaddr addr{};
        socklen_t len = sizeof(addr);
        cout<<strerror(errno)<<endl;
        int newFd = accept(fd_, &addr, &len);
        setSocketNonBlocking(newFd);

        cout<<strerror(errno)<<endl;
        if(newFd == -1)
            return;

        shared_ptr<EventLoop>&loop = subLoops[rand()%threadNum_];

        SP_Channel ch = make_shared<Channel>(newFd);
        shared_ptr<TcpConnection> conn = make_shared<TcpConnection>(loop, ch);
        connOfFd[newFd] = conn;
        ch->setOwner(conn);
        ch->setReadCallback( bind(&TcpConnection::handleRead, conn) );
        ch->setWriteCallback( bind(&TcpConnection::handleWrite, conn) );
        // close 部分由 write 处理？ todo
        loop->addChannel(std::move(ch));
    }

    void start(){
        for(int i=0; i<threadNum_; i++){
            subLoops[i]->start();
        }
        mainLoop.start();
        mainLoop.loop();
    }
};


#endif //TOYWEBSERVER_SERVER_H
