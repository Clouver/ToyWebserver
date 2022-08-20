//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_CHANNEL_H
#define TOYWEBSERVER_CHANNEL_H

#include<functional>
#include<memory>
#include <utility>
#include <sys/epoll.h>

using namespace std;

class Channel;
class TcpConnection;
typedef shared_ptr<Channel> SP_Channel;


// todo callback传裸指针解决循环引用问题，那么裸指针的对象在别的地方析构了怎么办？
// 当前Connection处理流程绝对不会出现这种不由channel引发(channel引发时立即kill自己就安全了）的析构的情况。
// 假如我的代码被其他人使用，则什么都有可能。那就暂不考虑。

// channel 绑定 fd 和 回调
// 不拥有任何资源！ fd 不由它释放。
class Channel{
    int fd_;
    bool alive;
    std::weak_ptr<TcpConnection>owner_; // 不拥有 fd 本身。 weakptr 防止循环

    // 不一定是epoll_wait 获得的events。
    // 例如 read 0 byte代表连接正常关闭，设置额外的EPOLLHUP去处理连接的关闭。
    uint32_t events;

    // 传入bind(callback, this) ;不用 connection的shared_ptr防止循环引用。
    function<void()>readCallback;
    function<void()>writeCallback;
    function<void()>closeCallback;

public:
    Channel() = delete;

    explicit Channel(int fd);

    bool isAlive() const;

    void kill();

    uint32_t getEvent() const;

    void setEvent(uint32_t e);

    int getfd() const;

    shared_ptr<TcpConnection> getOwner();
    void setOwner(std::weak_ptr<TcpConnection>owner);

    void setReadCallback(function<void()> func);
    void setWriteCallback(function<void()> func);
    void setCloseCallback(function<void()>func);

    void handleEvents();
};

#endif //TOYWEBSERVER_CHANNEL_H
