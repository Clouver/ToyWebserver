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

// todo 使用 char* 提高效率
ssize_t readAll(int fd, vector<char>& readTo);

// todo 使用 char* 提高效率
ssize_t writeAll(int fd, const string& writeFrom);

int createFdThenBindListen(int port, int backlog);

int setSocketNonBlocking(int fd);

#endif //TOYWEBSERVER_NETWORK_H
