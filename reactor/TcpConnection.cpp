//
// Created by Clouver on 2022/8/13.
//

#include "TcpConnection.h"

TcpConnection::TcpConnection(int fd, shared_ptr<EventLoop> eventLoop){
    eventLoop_ = eventLoop;
    fd_ = fd;
    channel = make_shared<Channel>(fd_);
    alive = true;

//    channel->setOwner(shared_from_this()); 构造没有完成!
    channel->setReadCallback( bind(&TcpConnection::handleRead, this) );
    channel->setWriteCallback( bind(&TcpConnection::handleWrite, this) );
    channel->setCloseCallback( bind(&TcpConnection::handleClose, this) );

    eventLoop_->addChannel(channel);

}
std::shared_ptr<Channel>& TcpConnection::getChannel(){
    return channel;
}

shared_ptr<EventLoop> TcpConnection::getLoop(){
    return eventLoop_;
}
void TcpConnection::setReadCallback(function<void()> func) {
    channel->setReadCallback(func);
}

void TcpConnection::setCloseCallback(function<void()> func) {
    channel->setCloseCallback(func);
}

void TcpConnection::handleRead(){

    ssize_t sz = readAll(fd_, buf);
    if(sz == 0){
        // close todo 这里把channel的状态设置为 EPOLLHUP。 会和真正的意外断开EPOLLHUP混淆吗？
        // channel handleEvents 时的顺序 read write close，所以在read设置close关闭连接。
        channel->setEvent(channel->getEvent()|EPOLLHUP);
        channel->kill();
        // handleRead 会成为 channel 的回调，在eventloop运行，kill是及时的不会导致处理已关闭的channel
        alive = false;
    }
    else if(sz == -1){
        channel->setEvent(channel->getEvent()|EPOLLHUP);
        channel->kill();
        alive = false;
    }
    else{
        // echo 服务 仅供测试 todo
//        for(int i=0; i<sz/2; i++)
//            swap(buf[i], buf[sz-i-1]);
//        write(fd_, buf.c_str(), sz);

//        cout<<buf<<endl;

        service.SolveRequest(channel->getfd(), buf);
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

};

// todo close(fd_) 有可能仍然在 readHandle 这边就关了
// 但是read返回-1，也能正常处理。
void TcpConnection::release(){
    // 取消注册
    // channel 存在的几个地方
    // 1、 TcpConnection.channel SP 一直存在
    // 2、 EventLoop.toAdd[] SP
    // 3、 Poller.ChannelOfFd[] SP

    channel->kill(); // 处理2
    eventLoop_->delChannel(channel); // 处理3
    channel.reset(); // 处理1

    close(fd_);
    fd_ = 0;
}

TcpConnection::~TcpConnection(){
    if(channel){
        // 未正常 release。
        // 正常应该在 handleClose 中调用了 release释放了channel
        release();
    }
    if(fd_){
        close(fd_);
    }
}