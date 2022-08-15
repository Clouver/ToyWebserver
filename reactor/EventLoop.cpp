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

        // 可能其他地方给当前loop添加了新的Channel，放在了toAdd里。
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
int EventLoop::bindThread(shared_ptr<std::thread>& t) {
    // 传入了 this， thread 通过 this 唯一绑定了一个 EventLoop
//    t = ;
    return 0;
}

int EventLoop::addChannel(shared_ptr<Channel>&& ch){
    if(!ch)
        return 0;
    unique_lock<mutex>lock(m);
    toAdd.push(std::move(ch));

    lock.unlock();
    cond.notify_one();
    return 0;
}