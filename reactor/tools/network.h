//
// Created by Clouver on 2022/8/15.
//

#ifndef TOYWEBSERVER_NETWORK_H
#define TOYWEBSERVER_NETWORK_H

#include <sys/socket.h>
#include <csignal>
#include <exception>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <vector>

using namespace std;

// todo 这里假设了一次能读完
ssize_t readAll(int fd, vector<char>& readTo);

// todo 这里假设一次能写完 按理说写了一部分 错误了 也该把已写的保留？
ssize_t writeAll(int fd, const string& writeFrom);

int createBindListen(int port, int backlog);

int setSocketNonBlocking(int fd);

#endif //TOYWEBSERVER_NETWORK_H
