//
// Created by Clouver on 2022/8/13.
//

#include "EventLoop.h"

EventLoop::EventLoop():poller(shared_ptr<Poller>(new Poller())), loop_(true){
}
void EventLoop::start(){
    loop_ = true;
}
void EventLoop::loop() {
    while(loop_){
        pollAndHandle();
        unique_lock<mutex>lock(m, std::defer_lock); // 设置 defer_lock 创建时不加锁
        lock.try_lock();
        if(lock.owns_lock()){
            while(!toAdd.empty()){
                poller->add(toAdd.front());
                toAdd.pop();
            }
            lock.unlock();
        }
    }
}


void EventLoop::pollAndHandle() {
    vector<SP_Channel> active = poller->poll();
    for(SP_Channel& ch : active){
        ch->handleEvents();
    }
}
int EventLoop::bindThread(shared_ptr<std::thread> t) {
    // 传入了 this， thread 通过 this 唯一绑定了一个 EventLoop
    t = shared_ptr<std::thread>(new thread(bind(&EventLoop::loop, this)));
}

int EventLoop::addChannel(Channel& ch){
    unique_lock<mutex>lock(m);
    toAdd.push(ch);
    lock.unlock();
    cond.notify_one();
}