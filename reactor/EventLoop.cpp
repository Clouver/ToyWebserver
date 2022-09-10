//
// Created by Clouver on 2022/8/13.
//

#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include "tools/Timer.h"

#include <memory>
#include <iostream>

// for strerror
#include <string.h>
#include <syscall.h>

using namespace std;

//chrono::steady_clock::time_point t1;
//chrono::steady_clock::time_point t2;
//t2 = chrono::steady_clock::now();
//cout<<chrono::duration_cast<chrono::duration<double>>(t2 - t1).count()<<endl;

const bool TIME_COST_DEBUG = true;
static const size_t TASK_QUEUE_SIZE = 4;

EventLoop::EventLoop():poller(make_shared<Poller>()), loop_(true), qTask(TASK_QUEUE_SIZE){

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
        if(TIME_COST_DEBUG){
            static Timer timer("pollAndHandle");
            // timer.runInTiming(bind(&EventLoop::pollAndHandle, this) );
            timer.tick();
            pollAndHandle();
            timer.tock();
        }
        else
            pollAndHandle();

        while(!qTask.empty()){
            qTask.front()();
            qTask.pop();
        }
    }
}

void EventLoop::pollAndHandle() {
    static Timer clear("\tclear"), poll("\tpoll "), handleEvent("\tloop ");
    clear.tick();
    active.clear();
    clear.tock();

    poll.tick();
    poller->poll(active);
    poll.tock();

    handleEvent.tick();
    for(SP_Channel* ch : active){
        (*ch)->handleEvents();
    }
    handleEvent.tock();
}

int EventLoop::pushTask(const function<void()> &f){
    if (qTask.push(f) != 0)
        return -1;
    wakeup();
    return 0;
}

void EventLoop::addChannel(const shared_ptr<Channel>& ch){
    ch->setRunner(shared_from_this());
    poller->add(ch);
}

int EventLoop::delChannel(SP_Channel& ch){
    ch->setCloseCallback(nullptr); // 消灭隐藏的connection； conn和channel 现在应当只剩 pconn / pconn->channel / ch
    return poller->del(ch);
}

// 固定使用 8 位！
void EventLoop::wakeupRead() const {
    uint64_t toread = 0;
    if(read(wakeupfd,&toread,8) == -1)
    {
        cout<<strerror(errno)<<endl;
    }
}

void EventLoop::closeWakeup() const{
    close(wakeupfd);
}

void EventLoop::wakeup() const{
    uint64_t towrite = 1;
    if(write(wakeupfd,&towrite,8) == -1)
    {
        cout<<strerror(errno)<<endl;
    }
}