//
// Created by Clouver on 2022/8/13.
//
#include<sys/epoll.h>
#include<vector>

using namespace std;

const int EPOLL_SIZE = 50;
const int EPOLL_MAX_EVENT = 10000;
const int EPOLL_TIMEOUT = 500;
class Poller{
    int epfd;
    vector<struct epoll_event>events;
public:
    Poller(){
        epfd = epoll_create(EPOLL_SIZE);
        events = vector<struct epoll_event>(EPOLL_MAX_EVENT);
    };

    vector<struct epoll_event> pollLoop(){
        vector<struct epoll_event>activeEvents;
        while(true){
            //
            int cnt = epoll_wait(epfd, &*events.begin(), EPOLL_MAX_EVENT, EPOLL_TIMEOUT);
            if(cnt > 0){
                for(int i=0; i<cnt; i++)
                    activeEvents.push_back(events[i]);
                break;
            }
        }
        return activeEvents;
    }

    int add(int fd){
        struct epoll_event e;
        e.data.fd = fd;
        e.events = EPOLLIN|EPOLLOUT;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &e);
        return ret;
    }
};