//
// Created by Clouver on 2022/8/13.
//

#ifndef TOYWEBSERVER_TCPCONNECTION_H
#define TOYWEBSERVER_TCPCONNECTION_H

#include <netinet/in.h>

class TcpConnection {
    int fd;
    struct sockaddr_in addrin;
public:

    ~TcpConnection(); // 析构拥有的资源 比如 fd
};


#endif //TOYWEBSERVER_TCPCONNECTION_H
