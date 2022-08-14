//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_POLLER_H
#define TOYWEBSERVER_POLLER_H
#include<vector>
#include "Channel.h"
#include <sys/epoll.h>
#include <unordered_map>

using namespace std;

class Poller {
    int pollfd;
    int max_events;
    vector<struct epoll_event> eventBuffer;

    unordered_map<int, SP_Channel>channelOfFd;

public:
    Poller();
    /* 初始 listenFd 对应的channel在server创建时创建，
     * 其他新 channel 在 poll 到 listenChannel 时，listenChannel->handleConn() 创建。
     */
    vector<SP_Channel> poll(){

        int cnt = epoll_wait(pollfd, &*eventBuffer.begin(), max_events, 0);
        vector<SP_Channel>active;
        for(int i=0; i<cnt; i++){
            active.push_back(channelOfFd[eventBuffer[i].data.fd]);
            eventBuffer[i].events = EPOLLIN|EPOLLOUT;
            epoll_ctl(pollfd, EPOLL_CTL_MOD, eventBuffer[i].data.fd, &eventBuffer[i]);
        }
        return active;
    }

    int add(SP_Channel& channel){
        channelOfFd[channel->getfd()] = channel;
    }
};


#endif //TOYWEBSERVER_POLLER_H
