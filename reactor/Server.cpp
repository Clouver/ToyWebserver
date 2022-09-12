//
// Created by Clouver on 2022/8/14.
//

#include "Server.h"

#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "tools/network.h"

#include <sys/socket.h>
#include <functional>
#include <fcntl.h>
#include <csignal>
#include <unistd.h>
#include <utility>
#include <netinet/tcp.h>
#include <cassert>
#include <iostream>


const int connBacklog = 200;

Server::Server():fd_(0), port_(0),
                 threadNum_(0),
                 maxConnSize_(0){};

shared_ptr<EventLoop> Server::getLoop() const {
    return mainLoop;
}

Server::Server(int port, int threadNum, int maxConnSize, shared_ptr<ServiceFactory> factory):fd_(0), port_(port),
                                                         threadNum_(threadNum),
                                                         maxConnSize_(maxConnSize),
                                                         mainLoop(nullptr),subLoops(), threads(),
                                                         connFact(std::move(factory))
                                                         {
    /*
     * Server 由几个部分组成：
     *      mainloop                即主reactor。这里不单开线程了，直接主线程进行loop。 也没有单独的 Acceptor，mainloop就是acceptor，只修改一下handleread。
     *      subLoops + threads      即从reactors。线程池。
     *      connOfFd                保存已建立的连接。（包含了acceptChannel所在的伪连接）
     *      connManager             负责释放已关闭的连接。
     */

}

void Server::runSubreactors(){
    for(int i=0; i<threadNum_; i++){
        subLoops.push_back(make_shared<EventLoop>());
        subLoops.back()->start(); // 必须先设置 start，再创建线程。
        threads.push_back( make_shared<std::thread>(bind(&EventLoop::loop, subLoops.back())) );
    }
}

void Server::setMainLoop(){
    fd_ = createFdThenBindListen(port_, maxConnSize_);

    if(fd_ == -1)
        throw exception();

    /*
     * AcceptChannel 的定义。
     *
     * acceptchannel 的 owner 为空
     */
    ch_ = make_shared<Channel>(fd_);
    ch_->setReadCallback( bind(&Server::handleNewConn, this));
    mainLoop = make_shared<EventLoop>();
    mainLoop->addChannel(ch_);
}

// 回调只在 eventloop 中执行则线程安全。
// todo 只能正确处理连接处理时关闭，其它线程无法关闭某连接，否则可能读写关闭的socket。
void Server::handleClose(TcpConnection *pconn){

    SP_Channel ch = pconn->getChannel();
    ch->getRunner()->delChannel(ch);

    pconn->release();

    connOfFd.erase(ch->getfd());
}

// 新连接
void Server::handleNewConn(){
    while(true){

        // todo 待封装成 acceptor
        sockaddr addr{};
        socklen_t len = sizeof(addr);
        int newFd = accept(fd_, &addr, &len);
        if(newFd <= 0)
            break;

        setSocketNonBlocking(newFd);

        int t=1;
        setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY,&t, sizeof t);

        shared_ptr<TcpConnection> conn = TcpConnectionFactory::create(newFd,
                                                                      reinterpret_cast<sockaddr_in*>(&addr),
                                                                      shared_from_this(),
                                                                      connFact);
        connOfFd[newFd] = conn;

        // 线程池中随机
        shared_ptr<EventLoop>&loop = subLoops[rand()%threadNum_];
//        loop->addChannel(conn->getChannel() ); // 入队列，等待处理 todo 无锁
        if (loop->pushTask(bind(&EventLoop::addChannel, loop, conn->getChannel()) ) != 0){
            conn->release();
            connOfFd.erase(newFd);
        }
    }
}

void Server::start(){

    setMainLoop();
    runSubreactors();

    mainLoop->start();
    mainLoop->loop();
}

Server::~Server(){
    for(int i=0; i<threadNum_; i++)
        subLoops[i]->stop(), subLoops[i].reset();
    for(int i=0; i<threadNum_; i++)
        threads[i]->join(), threads[i].reset();

    close(fd_);
}