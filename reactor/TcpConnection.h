//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_TCPCONNECTION_H
#define TOYWEBSERVER_TCPCONNECTION_H

#include <netinet/in.h>
#include <csignal>
#include "Channel.h"
#include <memory>
#include <sys/epoll.h>
#include "tools/network.h"

class EventLoop;
class Channel;

class TcpConnection {
    int fd;
    struct sockaddr_in addrin;
    std::shared_ptr<Channel> channel;

    shared_ptr<EventLoop> eventLoop_;

    std::string buf;
    std::string toWrite;

public:
    TcpConnection(shared_ptr<EventLoop> eventLoop, shared_ptr<Channel>& ch){
        eventLoop_ = eventLoop;
        channel = ch;
        fd = ch->getfd();
    }
    std::shared_ptr<Channel>& getChannel(){
        return channel;
    }
    void handleRead(){

        ssize_t sz = readal(fd, buf);
        if(sz == 0){
            // close todo
        }
        else if(sz == -1){
            // err close todo
        }
        else{
            // now just for test todo
            for(int i=0; i<sz/2; i++)
                swap(buf[i], buf[sz-i-1]);
            write(fd, buf.c_str(), sz);
        }
    }

    void handleWrite(){
        ssize_t sz = writeAl(fd, toWrite);
        if(sz == 0){
            //
        }
        else if(sz == toWrite.size()){
        }
        else if(sz < toWrite.size()){
            // todo 剩余
            toWrite = toWrite.substr(sz, toWrite.size()-sz);
            channel->setEvent(EPOLLOUT);
        }
    }

    void handleClose(){
        close(fd);

    }

    // 析构拥有的资源 比如 fd
    ~TcpConnection(){
        close(fd);
    }
};


#endif //TOYWEBSERVER_TCPCONNECTION_H
