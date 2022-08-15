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


using namespace std;

class EventLoop {
    mutex m;                // protect cond
    condition_variable cond; // for queue of new channel to add
    queue<shared_ptr<Channel>>toAdd;    // new channel to add


    bool loop_;

public:
    shared_ptr<Poller>poller;

    EventLoop();

    void start();
    void loop(); // 无限循环 pollAndHandle
    void pollAndHandle(); // handleConn 时会产生新的 connection、channel、acceptFd。

    // one loop one thread 实际上是 one thread one loop，loop整个交给thread了，而不是loop在调度thread
    // 把pollAndHandle 连带 this 当成 threadfunc 传进去。
    // 调用时机是 sever 初始化线程池，就一个一个加进去。
    int bindThread(shared_ptr<thread>& t);

    // channel 加给 poller，loop 时
    int addChannel(shared_ptr<Channel>&& ch);
};

#endif //TOYWEBSERVER_EVENTLOOP_H
