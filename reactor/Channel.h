//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_CHANNEL_H
#define TOYWEBSERVER_CHANNEL_H


#include<functional>
#include<memory>
#include"TcpConnection.h"

using namespace std;


class Channel;

typedef shared_ptr<Channel> SP_Channel;

// channel 绑定 fd 和 回调
class Channel{
    int fd_;
    shared_ptr<TcpConnection>owner; // 不拥有 fd 本身。 weakptr?

    function<void()>readCallback;
    function<void()>writeCallback;
    function<void()>connCallback;
public:
    Channel():Channel(0){};
    Channel(int fd):fd_(fd), readCallback(0), writeCallback(0), connCallback(0){
    };
    int getfd(){
        return fd_;
    }
    void setfd(int fd){
        fd_ = fd;
    }
    void setReadCallback(function<void()> func){
        readCallback = func;
    }
    void setWriteCallback(function<void()> func){
        readCallback = func;
    }
    void setConnCallback(function<void()> func){
        readCallback = func;
    }
    void handleEvents(){
        if(readCallback)
            readCallback();
        if(writeCallback)
            writeCallback();
        if(connCallback)
            connCallback();
    }
};

class ListenChannel : Channel{

};

#endif //TOYWEBSERVER_CHANNEL_H
