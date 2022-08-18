//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_EVENTLOOP_H
#define TOYWEBSERVER_EVENTLOOP_H

#include "Poller.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <sys/eventfd.h>

using namespace std;

class Poller;
class Channel;

class EventLoop {
    mutex m;                //
    condition_variable cond; // 保护新增channel的队列
    queue<shared_ptr<Channel>>toAdd;    // 新增队列

    bool loop_;

    SP_Channel wakeupCh;
    int wakeupfd;
public:
    shared_ptr<Poller>poller;

    EventLoop();

    void start();
    void stop();
    void loop(); // 无限循环 pollAndHandle
    void pollAndHandle(); // handleConn 时会产生新的 connection、channel、acceptFd。

    // one loop one thread 实际上是 one thread one loop，loop整个交给thread了，而不是loop在调度thread
    // 把pollAndHandle 连带 this 当成 threadfunc 传进去。
    // 调用时机是 sever 初始化线程池，就一个一个加进去。
    int bindThread(shared_ptr<thread>& t);

    // channel 加给 poller，loop 时
    int addChannel(shared_ptr<Channel> ch);

    int delChannel(SP_Channel& ch);

    // 用于 wakeupCh 的绑定，仅仅用来解除 epoll_wait 的阻塞。
    void wakeupRead();
    void closeWakeup();
    void wakeup();
};

#endif //TOYWEBSERVER_EVENTLOOP_H
