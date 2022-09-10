//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_POLLER_H
#define TOYWEBSERVER_POLLER_H
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

using namespace std;

class Channel;
typedef shared_ptr<Channel> SP_Channel;


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
    void poll(vector<SP_Channel*>& active );

    int add(const SP_Channel& channel);

    int del(const SP_Channel& channel);
};

#endif //TOYWEBSERVER_POLLER_H
