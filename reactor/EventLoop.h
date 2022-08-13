//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_EVENTLOOP_H
#define TOYWEBSERVER_EVENTLOOP_H

#include "Poller.h"
#include <memory>

using namespace std;

class EventLoop {
    shared_ptr<Poller>poller;

public:
    void loop(); // 无限循环 pollAndHandle
    void pollAndHandle(); // handleConn 时会产生新的 connection、channel、acceptFd。
};


#endif //TOYWEBSERVER_EVENTLOOP_H
