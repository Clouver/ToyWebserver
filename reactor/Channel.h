//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_CHANNEL_H
#define TOYWEBSERVER_CHANNEL_H


#include<functional>
#include<memory>
#include <sys/epoll.h>

using namespace std;


class Channel;
class TcpConnection;
typedef shared_ptr<Channel> SP_Channel;

// channel 绑定 fd 和 回调
// 不拥有任何资源！ fd 不由它释放。
// 注意绑定 callback 不要传 shared_ptr 可能导致循环引用无法析构
// todo callback传裸指针解决循环引用问题，那么裸指针的对象在别的地方析构了怎么办？

class Channel{
    int fd_;
    bool alive;
    std::weak_ptr<TcpConnection>owner_; // 不拥有 fd 本身。 weakptr 防止循环

    // 不一定是epoll_wait 获得的events。
    // 例如 read 0 byte代表连接正常关闭，设置额外的EPOLLHUP去处理连接的关闭。
    uint32_t events;

    function<void()>readCallback;
    function<void()>writeCallback;
    function<void()>closeCallback;

public:
    Channel():Channel(0){};
    Channel(int fd):fd_(fd),alive(true), readCallback(nullptr), writeCallback(nullptr), closeCallback(nullptr){
    };

    bool isAlive(){
        return alive;
    }
    void kill(){
        alive = false;
    }

    uint32_t getEvent(void){
        return events;
    }
    void setEvent(uint32_t e){
        events = e;
    }

    int getfd(){
        return fd_;
    }
    void setfd(int fd){
        fd_ = fd;
    }

    shared_ptr<TcpConnection> getOwner(){
        return owner_.lock();
    }
    void setOwner(std::weak_ptr<TcpConnection>&&owner){
        owner_ = owner;
    }

    void setReadCallback(function<void()> func){
        readCallback = func;
    }
    void setWriteCallback(function<void()> func){
        writeCallback = func;
    }
    void setCloseCallback(function<void()>func){
        closeCallback = func;
    }

    void handleEvents(){
        if(events&EPOLLIN && readCallback)
            readCallback();
        if(events&EPOLLOUT && writeCallback)
            writeCallback();
        if(events&EPOLLHUP && closeCallback)
            closeCallback();
    }
};

#endif //TOYWEBSERVER_CHANNEL_H
