//
// Created by Clouver on 2022/8/13.
//

#include "Poller.h"
#include "Channel.h"

#include <sys/epoll.h>
#include <cstring>
#include <csignal>
#include <iostream>
//#include <cerrno>
//#include <string>
//#include <iostream>

int EPOLL_WAIT_TIMEOUT = 100000;

Poller::Poller(){
    max_events = 1000; // todo
    pollfd = epoll_create1(EPOLL_CLOEXEC);
    int errnoSave = errno;
    if(pollfd == -1){
        cout<<"epoll create failed: "<<strerror(errno)<<endl;
    }
    eventBuffer = vector<struct epoll_event>(max_events);
};

Poller::~Poller(){
    close(pollfd);
}

/* 初始 listenFd 对应的channel在server创建时创建，
 * 其他新 channel 在 poll 到 listenChannel 时，listenChannel->handleNewConn() 创建。
 */
void Poller::poll(vector<SP_Channel*>& active ){

    int cnt = epoll_wait(pollfd, &*eventBuffer.begin(), max_events, EPOLL_WAIT_TIMEOUT);
    int errnoSave = errno;
    if(cnt == -1){
        cout<<strerror(errnoSave)<<endl;
    }
    for(int i=0; i<cnt; i++){
        int curfd = eventBuffer[i].data.fd;
        SP_Channel* cur = &channelOfFd[curfd];
        (*cur)->setEvent(eventBuffer[i].events);
        active.push_back(cur);
    }
}

int Poller::add(const SP_Channel& channel){

    channelOfFd[channel->getfd()] = channel;
    epoll_event e{};
    memset(&e, 0, sizeof(e));
    // 坑点 epoll_ctl 的参数有fd，但epollevent里也必须设置好fd
    // 因为 epoll_wait 只会修改 events，而不管 epollevent 里面其他值，导致拿到epollevent却不知道是哪个fd引起的。
    e.data.fd = channel->getfd();
    e.events = EPOLLIN|EPOLLHUP;//|EPOLLET;
    epoll_ctl(pollfd, EPOLL_CTL_ADD, channel->getfd(), &e);

    return 0;
}

int Poller::del(const SP_Channel& channel){
    int fd = channel->getfd();
//    unique_lock<mutex>lock(m);
    if(channelOfFd.count(fd)== 0)
        return -1;
    channelOfFd.erase(fd);
    epoll_event e{};
    epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, &e);

    return 0;
}
