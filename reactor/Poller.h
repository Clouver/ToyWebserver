//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_POLLER_H
#define TOYWEBSERVER_POLLER_H
#include<vector>
#include "Channel.h"
#include <sys/epoll.h>
#include <unordered_map>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <string>
#include <iostream>
#include "TcpConnection.h"
#include <mutex>
#include <unistd.h>

using namespace std;

class Poller {
    int pollfd;
    int max_events;
    vector<struct epoll_event> eventBuffer;

    mutex m;
    unordered_map<int, SP_Channel>channelOfFd;

public:
    Poller();
    ~Poller();
    int getPollFd();

    /* 初始 listenFd 对应的channel在server创建时创建，
     * 其他新 channel 在 poll 到 listenChannel 时，listenChannel->handleConn() 创建。
     */
    vector<SP_Channel> poll();

    int add(SP_Channel& channel);

    int del(int fd);
};

#endif //TOYWEBSERVER_POLLER_H
