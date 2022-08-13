//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_POLLER_H
#define TOYWEBSERVER_POLLER_H
#include<vector>
#include "Channel.h"
#include <sys/epoll.h>

using namespace std;

class Poller {
    int pollfd;
    int max_events;
    shared_ptr<struct epoll_event> eventBuffer;

public:
    Poller();
    /* 初始 listenFd 对应的channel在server创建时创建，
     * 其他新 channel 在 poll 到 listenChannel 时，listenChannel->handleConn() 创建。
     */
    vector<SP_Channel> poll();

    int add(Channel& channel);
};


#endif //TOYWEBSERVER_POLLER_H
