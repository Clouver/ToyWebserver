//
// Created by Clouver on 2022/8/13.
//

#include "Poller.h"


int EPOLL_WAIT_TIMEOUT = 50;

Poller::Poller(){
    max_events = 100; // todo
    pollfd = epoll_create(max_events);
    eventBuffer = vector<struct epoll_event>(max_events);
};
Poller::~Poller(){
    close(pollfd);
}
int Poller::getPollFd(){
    return pollfd;
}

/* 初始 listenFd 对应的channel在server创建时创建，
 * 其他新 channel 在 poll 到 listenChannel 时，listenChannel->handleConn() 创建。
 */
vector<SP_Channel> Poller::poll(){

    int cnt = epoll_wait(pollfd, &*eventBuffer.begin(), max_events, EPOLL_WAIT_TIMEOUT);
    //string s(strerror(errno));
    vector<SP_Channel>active;

//    if(cnt > 0)
//        cout<<cnt<<" RWC: "<<(eventBuffer[0].events&EPOLLIN)<<" "<<(eventBuffer[0].events&EPOLLOUT)<<" "<<(eventBuffer[0].events&EPOLLHUP)<<endl;

    for(int i=0; i<cnt; i++){
        int curfd = eventBuffer[i].data.fd;
        // todo 效率应该会很低
        unique_lock<mutex>lock(m);
        if(channelOfFd.count(curfd) == 0)
            // 连接关闭是由单独的线程处理的，可能会迟一点导致仍然接收到 0byte 的 EPOLLIN
            // todo 有没有更好的处理办法？
            continue;
        if(!channelOfFd[curfd]->isAlive())
            continue;
        SP_Channel cur = channelOfFd[eventBuffer[i].data.fd];
        cur->setEvent(eventBuffer[i].events);
        active.push_back(cur);
    }

    return active;
}

int Poller::add(SP_Channel& channel){
    unique_lock<mutex>lock(m);

    channelOfFd[channel->getfd()] = channel;
//    connOfFd[channel->getfd()] = channel->getOwner();

    epoll_event e{};
    memset(&e, 0, sizeof(e));
    // 坑点 epoll_ctl 的参数有fd，但epollevent里也必须设置好fd
    // 因为 epoll_wait 只会修改 events，而不管 epollevent 里面其他值，导致拿到epollevent却不知道是哪个fd引起的。
    e.data.fd = channel->getfd();
    e.events = EPOLLIN|EPOLLHUP;//|EPOLLET;
    epoll_ctl(pollfd, EPOLL_CTL_ADD, channel->getfd(), &e);
    return 0;
}


int Poller::del(int fd){
    unique_lock<mutex>lock(m);
    if(channelOfFd.count(fd)== 0)
        return -1;
    channelOfFd.erase(fd);
    epoll_event e{};
    epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, &e);
}
