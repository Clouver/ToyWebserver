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

    // 一次 epoll_wait
    // todo buffer 取代 值传递的vector 应该能很好提高效率; poll( vector<>& )？
    vector<SP_Channel> poll();

    int add(const SP_Channel& channel);

    int del(const SP_Channel& channel);
};

#endif //TOYWEBSERVER_POLLER_H
