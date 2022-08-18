//
// Created by Clouver on 2022/8/13.
//

#include "EventLoop.h"

//chrono::steady_clock::time_point t1;
//chrono::steady_clock::time_point t2;
//t2 = chrono::steady_clock::now();
//cout<<chrono::duration_cast<chrono::duration<double>>(t2 - t1).count()<<endl;

EventLoop::EventLoop():poller(shared_ptr<Poller>(new Poller())), loop_(true){

    // eventfd 非阻塞
    wakeupfd = eventfd(0, EFD_NONBLOCK);
    wakeupCh = make_shared<Channel>(wakeupfd);
    wakeupCh->setReadCallback(bind(&EventLoop::wakeupRead, this));
    wakeupCh->setCloseCallback( bind(&EventLoop::closeWakeup, this));

    poller->add(wakeupCh);
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

int EventLoop::addChannel(shared_ptr<Channel> ch){
    if(!ch)
        return -1;

    unique_lock<mutex>lock(m);
    toAdd.push(std::move(ch));
    lock.unlock();
    wakeup();
    return 0;
}

int EventLoop::delChannel(SP_Channel& ch){
    return poller->del(ch->getfd());
}

// 固定使用 8 位！
void EventLoop::wakeupRead() {
    uint64_t toread = 0;
    if(read(wakeupfd,&toread,8) == -1) //
    {
        perror(NULL);
        cout<<strerror(errno)<<endl;
    }
}

void EventLoop::closeWakeup(){
    close(wakeupfd);
}

void EventLoop::wakeup(){
    uint64_t towrite = 1;
    if(write(wakeupfd,&towrite,8) == -1) //
    {
        perror(NULL);
        cout<<strerror(errno)<<endl;
    }
}