//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_EVENTLOOP_H
#define TOYWEBSERVER_EVENTLOOP_H

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <sys/eventfd.h>
#include <unistd.h>
#include "tools/CircleQueue.h"

using namespace std;

class Poller;
class Channel;
typedef shared_ptr<Channel> SP_Channel;

class EventLoop : public std::enable_shared_from_this<EventLoop> {
    mutex m;                //
    condition_variable cond; // 保护新增channel的队列
    queue<shared_ptr<Channel>>toAdd;    // 新增队列
    CircleQueue<function<void()>>qTask;

    bool loop_;

    SP_Channel wakeupCh;
    int wakeupfd;

    shared_ptr<Poller>poller;
    vector<SP_Channel*>active;
public:

    EventLoop();

    void start();
    void stop();

    void loop(); // 无限循环 pollAndHandle

    void pollAndHandle(); // handleNewConn 时会产生新的 connection、channel、acceptFd。

    int pushTask(const function<void()> &f);

    // channel 加给 poller，loop 时
    void addChannel(const shared_ptr<Channel>& ch);

    int delChannel(SP_Channel& ch);

    // 用于 wakeupCh 的绑定，仅仅用来解除 epoll_wait 的阻塞。
    void wakeupRead() const;
    void closeWakeup() const;
    void wakeup() const;
};

#endif //TOYWEBSERVER_EVENTLOOP_H
