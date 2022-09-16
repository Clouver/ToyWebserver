//
// Created by Clouver on 2022/8/14.
//

#include "Server.h"

#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "tools/network.h"
#include "tools/systools.h"

#include <sys/socket.h>
#include <functional>
#include <fcntl.h>
#include <csignal>
#include <unistd.h>
#include <utility>
#include <netinet/tcp.h>
#include <cassert>
#include <iostream>

// for setrlimit. fd limit
#include <sys/resource.h>
#include <cstring>

const int connBacklog = 200;


shared_ptr<EventLoop> Server::getLoop() const {
    return mainLoop;
}

Server::Server(int port, int threadNum, int maxConnSize, shared_ptr<ServiceFactory> factory):mainLoop(nullptr), subLoops(),
                                                         threads(),
                                                         fd_(0),
                                                         port_(port),
                                                         threadNum_(threadNum),
                                                         maxConnSize_(maxConnSize),connFact(std::move(factory)), connSet(maxConnSize*2)
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

//    for(auto it=connOfFd.begin(); it!=connOfFd.end(); it++){
//        cout<< it->first <<" ";
//    }
//    cout<<endl;
//    cout<<pconn->getName()<<endl;
    int fdSave = pconn->getChannel()->getfd();
    auto connSave = connSet[fdSave];//connOfFd[pconn->getName()]; // 增加一个引用，避免析构函数在 lockguard 内调用，减少持有锁的时间
    pconn->release();

//    lock_guard<mutex>lock(connMutex);
//    connOfFd.erase(pconn->getName());
    connSet.erase(fdSave);
}

// 新连接
void Server::handleNewConn(){
    while(true){

        // todo 待封装成 acceptor

        // make connection
        shared_ptr<TcpConnection> conn = TcpConnectionFactory::acceptAndCreate(fd_,
                                                                      shared_from_this(),
                                                                      connFact);
        if(!conn) {
//            cout<<"failed to accept new connection"<<endl;
            continue;
        }

        // add to connSet
        if(connSet.insert(conn) == -1){
            conn->release();
            return;
        }

        // 线程池中随机
        static int idx=0;
        shared_ptr<EventLoop>&loop = subLoops[idx++%threadNum_];
        if (loop->pushTask(bind(&EventLoop::addChannel, loop, conn->getChannel()) ) != 0){
            connSet.erase(conn);
            conn->release();
        }
    }
}

void Server::start(){

    setMainLoop();
    runSubreactors();

    setNOFILE(409600);

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

Server::Server():threadNum_(0),fd_(0),port_(),maxConnSize_(0),connSet(1){

}
