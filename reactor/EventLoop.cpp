//
// Created by Clouver on 2022/8/13.
//

#include "EventLoop.h"

EventLoop::EventLoop():poller(shared_ptr<Poller>(new Poller())), loop_(true){

    // eventfd 非阻塞
    wakeupfd = eventfd(0, EFD_NONBLOCK);
    wakeupCh = make_shared<Channel>(wakeupfd);
    wakeupCh->setReadCallback(bind(&EventLoop::wakeupRead, this));
    wakeupCh->setCloseCallback( bind(&EventLoop::closeWakeup, this));

    addChannel(wakeupCh);
}
void EventLoop::start(){
    loop_ = true;
}
void EventLoop::stop(){
    loop_ = false;
    // todo 阻塞在 loop 循环里，就不会查看 loop_的值。要传入信号将其唤醒。
}
void EventLoop::loop() {
    while(loop_){
        pollAndHandle();

        // 可能其他地方给当前loop添加了新的Channel，放在了toAdd里。
        unique_lock<mutex>lock(m, std::defer_lock); // 设置 defer_lock 创建时不加锁
        lock.try_lock();
        if(lock.owns_lock()){
            while(!toAdd.empty()){

                // channel 的生命周期归 TcpConnection 管， conn调用release从所有保存channel的地方删除channel；
                // 但是，toAdd 作为queue 并不好找到，所以这里加额外处理。
                // todo 解决方式不太简洁
                if(toAdd.front()->isAlive())
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

int EventLoop::addChannel(shared_ptr<Channel> ch){
    if(!ch)
        return -1;
    unique_lock<mutex>lock(m);
    toAdd.push(std::move(ch));

    lock.unlock();
    // todo 发出信号 如果poll阻塞住了可以醒来
    wakeup();

    return 0;
}


int EventLoop::delChannel(SP_Channel& ch){
    poller->del(ch->getfd());
}


void EventLoop::wakeupRead() {
    vector<char>one(1);
    int n = readal(wakeupfd, one);
}

void EventLoop::closeWakeup(){
    close(wakeupfd);
}

void EventLoop::wakeup(){
    writeAl(wakeupfd, "1" );
}