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

using namespace std;

class Poller {
    int pollfd;
    int max_events;
    vector<struct epoll_event> eventBuffer;

    unordered_map<int, SP_Channel>channelOfFd;
    unordered_map<int, shared_ptr<TcpConnection>>connOfFd;


public:
    Poller(){
        max_events = 100; // todo
        pollfd = epoll_create(max_events);
        eventBuffer = vector<struct epoll_event>(max_events);
    };
    ~Poller(){
        close(pollfd);
    }
    int getPollFd(){
        return pollfd;
    }

    /* 初始 listenFd 对应的channel在server创建时创建，
     * 其他新 channel 在 poll 到 listenChannel 时，listenChannel->handleConn() 创建。
     */
    vector<SP_Channel> poll(){

        int cnt = epoll_wait(pollfd, &*eventBuffer.begin(), max_events, 0);
        string s(strerror(errno));
        vector<SP_Channel>active;
        if(cnt > 0)
            cout<<cnt<<endl;
        for(int i=0; i<cnt; i++){
            if(eventBuffer[i].data.fd == 0)
                continue; // todo why 0?
            active.push_back(channelOfFd[eventBuffer[i].data.fd]);
//            eventBuffer[i].events = EPOLLIN|EPOLLOUT|EPOLLHUP;
//            epoll_ctl(pollfd, EPOLL_CTL_MOD, eventBuffer[i].data.fd, &eventBuffer[i]);
        }
        return active;
    }

    int add(SP_Channel& channel){
        channelOfFd[channel->getfd()] = channel;
        connOfFd[channel->getfd()] = channel->getOwner();

        epoll_event e{};
        memset(&e, 0, sizeof(e));
        // 坑点 epoll_ctl 的参数有fd，但epollevent里也必须设置好fd
        // 因为 epoll_wait 只会修改 events，而不管 epollevent 里面其他值，导致拿到epollevent却不知道是哪个fd引起的。
        e.data.fd = channel->getfd();
        e.events = EPOLLIN|EPOLLHUP;//|EPOLLET;
        epoll_ctl(pollfd, EPOLL_CTL_ADD, channel->getfd(), &e);
        return 0;
    }
};


#endif //TOYWEBSERVER_POLLER_H
