//
// Created by Clouver on 2022/8/13.
//

#include <csignal>
#include <memory>
#include <sys/epoll.h>
#include <iostream>

#include "TcpConnection.h"
#include "Server.h"

#include "Channel.h"
#include "EventLoop.h"

#include "tools/network.h"
#include "tools/Timer.h"

shared_ptr<TcpConnection> TcpConnectionFactory::create(int fd, const sockaddr_in* addr, const shared_ptr<Server>& server,
                                                       const shared_ptr<ServiceFactory>& serviceFact
                                                       ){
    shared_ptr<TcpConnection> conn = make_shared<TcpConnection>(fd, addr);
    serviceFact->create(conn->service);

    SP_Channel channel = conn->getChannel();

    channel->setOwner(conn);

    // 传入原指针避免循环引用
    channel->setReadCallback(  bind(&TcpConnection::handleRead , conn.get()) );
    channel->setWriteCallback( bind(&TcpConnection::handleWrite, conn.get()) );
    channel->setCloseCallback( bind(&Server::handleClose, server.get(), conn) );

    return std::move(conn);
}


TcpConnection::TcpConnection(int fd, const sockaddr_in* addr)
:fd_( fd ), alive(true), addrin(*addr), service(), channel(make_shared<Channel>(fd)){
}

std::shared_ptr<Channel> TcpConnection::getChannel(){
    return channel;
}

//shared_ptr<EventLoop> TcpConnection::getLoop(){
//    return eventLoop_;
//}
//void TcpConnection::setReadCallback(function<void()> func) {
//    channel->setReadCallback(func);
//}
//
//void TcpConnection::setCloseCallback(function<void()> func) {
//    channel->setCloseCallback(func);
//}

void TcpConnection::handleRead(){
    static Timer timer("\t\tread");
    timer.tick();
    buf.clear();
    ssize_t sz = readAll(fd_, buf);
    timer.tock();
    if(sz == 0){
        // close todo 这里把channel的状态设置为 EPOLLHUP。 会和真正的意外断开EPOLLHUP混淆吗？
        // channel handleEvents 时的顺序 read write close，所以在read设置close关闭连接。
        channel->setEvent(channel->getEvent()|EPOLLHUP);
//        channel->kill();
        // handleRead 会成为 channel 的回调，在eventloop运行，kill是及时的不会导致处理已关闭的channel
//        alive = false;
    }
    else if(sz == -1){
        channel->setEvent(channel->getEvent()|EPOLLHUP);
//        channel->kill();
//        alive = false;
    }
    else{
        service->SolveRequest(channel->getfd(), buf);
    }
}


void TcpConnection::handleWrite(){
    ssize_t sz = writeAll(fd_, toWrite);
    if(sz == 0||sz == toWrite.size()){
        channel->setEvent(channel->getEvent()&(~EPOLLOUT) );
    }
    else if(sz < toWrite.size()){
        // todo 剩余
        toWrite = toWrite.substr(sz, toWrite.size()-sz);
        channel->setEvent(EPOLLOUT);
    }
}

void TcpConnection::handleClose(){
    std::cout<<"close"<<std::endl;
    getChannel()->kill();
    const auto& ploop = getChannel()->getRunner();
    ploop->pushTask( bind(&EventLoop::delChannel, ploop, getChannel()) );
    ploop->wakeup();
};

// todo close(fd_) 有可能仍然在 readHandle 这边就关了
// 但是read返回-1，也能正常处理。
void TcpConnection::release(){
    // 取消注册
    // channel 存在的几个地方
    // 1、 TcpConnection.channel SP 一直存在
    // 2、 EventLoop.toAdd[] SP
    // 3、 Poller.ChannelOfFd[] SP
    if(channel){
        channel->kill(); // 处理2
//    channel->getRunner()->delChannel(channel); // 处理3  todo eventloop所在线程执行，则不会和poller竞争锁。
        channel->setCloseCallback(nullptr);
        channel->setReadCallback(nullptr);
        channel->setWriteCallback(nullptr);
        channel.reset(); // 处理1
    }
    if(fd_){
        close(fd_);
        fd_ = 0;
    }
}

TcpConnection::~TcpConnection(){
    release();
}