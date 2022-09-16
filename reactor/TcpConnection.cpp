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
    conn->service = serviceFact->create();

    SP_Channel channel = conn->getChannel();
    channel->setOwner(conn);

    // 传入原指针避免循环引用
    channel->setReadCallback(  bind(&TcpConnection::handleRead , conn.get()) );
    channel->setWriteCallback( bind(&TcpConnection::handleWrite, conn.get()) );
    channel->setCloseCallback( bind(&Server::handleClose, server.get(), conn.get()) );

    return std::move(conn);
}

shared_ptr<TcpConnection> TcpConnectionFactory::acceptAndCreate(int fd, const shared_ptr<Server>& server,
                                                       const shared_ptr<ServiceFactory>& serviceFact
){
    // accept
    sockaddr addr{};
    socklen_t len = sizeof(addr);
    int newFd = accept(fd, &addr, &len);
    int saveErrno = errno;
    if(newFd <= 0){
        cout<<"failed accept :"<<strerror(saveErrno)<<endl;
        return nullptr;
    }

    // set options
    setSocketNonBlocking(newFd);
    setTcpNoDelay(newFd);

    return TcpConnectionFactory::create(newFd, reinterpret_cast<const sockaddr_in *>(&addr), server, serviceFact);
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
        // channel handleEvents 时的顺序 read write close，所以在read设置close关闭连接。
        channel->setEvent(channel->getEvent()|EPOLLHUP);
    }
    else if(sz == -1){
        channel->setEvent(channel->getEvent()|EPOLLHUP);
    }
    else{
        if (service->SolveRequest(channel->getfd(), buf) != 0){
            cout<<"solve failed"<<endl;
            channel->setEvent(channel->getEvent()|EPOLLHUP);
        }
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
//    std::cout<<"close"<<std::endl;
//    getChannel()->kill();
//    const auto& ploop = getChannel()->getRunner();
//    ploop->pushTask( bind(&EventLoop::delChannel, ploop, getChannel()) );
//    ploop->wakeup();
    // todo
};


void TcpConnection::release(){

    if(channel){
        channel->kill();
        channel.reset();
    }

    if(fd_){
        shutdown(fd_, SHUT_WR);
//        close(fd_); // todo 用rst保证立即关闭？
        fd_ = 0;
    }
}

TcpConnection::~TcpConnection(){
    release(); // 无法等自己 release，channel 存在循环引用延长生命周期。
}

