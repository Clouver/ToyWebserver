//
// Created by Clouver on 2022/8/13.
//

#include "Channel.h"

Channel::Channel(int fd):fd_(fd), alive(true), events(0),
                            readCallback(nullptr),
                            writeCallback(nullptr),
                            closeCallback(nullptr){

}

bool Channel::isAlive() const{
    return alive;
}

void Channel::kill(){
    alive = false;
    setCloseCallback(nullptr); // 循环引用
    setReadCallback(nullptr);
    setWriteCallback(nullptr);
}

uint32_t Channel::getEvent() const{
    return events;
}

void Channel::setEvent(uint32_t e){
    events = e;
}

int Channel::getfd() const{
    return fd_;
}

//shared_ptr<TcpConnection> Channel::getOwner(){
//    return std::move(owner_.lock());
//}

void Channel::setOwner(std::weak_ptr<TcpConnection>owner){
    owner_ = std::move(owner);
}


shared_ptr<EventLoop> Channel::getRunner(){
    return runner_.lock();
}

void Channel::setRunner(shared_ptr<EventLoop> eventLoop){
    runner_ = eventLoop;
}

void Channel::setReadCallback(function<void()> func){
    readCallback = std::move(func);
}
void Channel::setWriteCallback(function<void()> func){
    writeCallback = std::move(func);
}
void Channel::setCloseCallback(function<void()>func){
    closeCallback = std::move(func);
}

void Channel::handleEvents(){
    if(events&EPOLLIN && readCallback)
        readCallback();
    if(events&EPOLLOUT && writeCallback)
        writeCallback();
    if(events&EPOLLHUP && closeCallback)
        closeCallback();
}
