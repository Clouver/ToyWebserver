//
// Created by Clouver on 2022/9/12.
//

#ifndef TOYWEBSERVER_CONNECTIONSET_H
#define TOYWEBSERVER_CONNECTIONSET_H

#include "TcpConnection.h"
#include "Channel.h"
#include <memory>
#include <utility>
#include <iostream>
#include <cstring>

using namespace std;


class ConnectionSet {
private:
    size_t sz_;
    shared_ptr<TcpConnection> *pConn;
    int *pFd;
    std::hash<int> intHash;

    size_t hash_(int x){
        return intHash(x);
    }

public:
    ConnectionSet() = delete;
    explicit ConnectionSet(size_t size):sz_(size), pConn(new shared_ptr<TcpConnection>[size]), pFd(new int[size]){
        memset(pFd, 0, size*sizeof(*pFd));
    }

    shared_ptr<TcpConnection> operator[](int fd){
        size_t i = hash_(fd), d=0;
        while(d < sz_ && pFd[(i+d)%sz_] != fd)
            d++;
        if(d < sz_ && pFd[(i+d)%sz_] == fd)
            return pConn[(i+d)%sz_];

        cout<<"failed erase"<<endl;
        return nullptr;
    }

    int erase(int fd){
        size_t i = hash_(fd), d=0;
        while(d < sz_ && pFd[(i+d)%sz_] != fd)
            d++;
        if(d < sz_ && pFd[(i+d)%sz_] == fd){
            pFd[(i+d)%sz_] = 0;
            pConn[(i+d)%sz_].reset();
            return 0;
        }
        cout<<"failed erase"<<endl;

        return -1;
    }

    int erase(shared_ptr<TcpConnection>& spConn){
        if(!spConn){
            cout<<"erase failed\n";
            return -1;
        }
        return erase(spConn->getChannel()->getfd());
    }

    int insert(shared_ptr<TcpConnection> spConn){
        int fd = spConn->getChannel()->getfd();
        size_t i = hash_(fd), d=0;
        while(d < sz_ && pFd[(i+d)%sz_] != 0)
            d++;
        if(d < sz_ && pFd[(i+d)%sz_] == 0){
            pFd[(i+d)%sz_] = fd;
            pConn[(i+d)%sz_] = std::move(spConn);
            return 0;
        }
        cout<<"failed insertion"<<endl;
        return -1;
    }

    ~ConnectionSet(){
        delete [] pConn;
        delete [] pFd;
    }
};


#endif //TOYWEBSERVER_CONNECTIONSET_H
