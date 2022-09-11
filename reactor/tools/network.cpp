//
// Created by Clouver on 2022/8/15.
//
#include "network.h"
#include "Timer.h"
#include "Buffer.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>


static const int BUFFER_SZ = 4096;

ssize_t readAll(int fd, Buffer& readTo){

    char buf[BUFFER_SZ];
    ssize_t sum=0, once=0;
    while(true) {
        static Timer readOnce("\t\t\treadOnce");
        readOnce.tick();
        once = read(fd, buf, BUFFER_SZ);
        readOnce.tock();

        if (once == -1) {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                return sum;
//            else if(errno == )
        }
        else if (once == 0)
            return sum;
        else {
            static Timer append("\t\t\tappend");
            append.tick();
                sum+=once;
                buf[once] = 0;
                readTo.append(buf, once);
            append.tock();
        }
    }
}

//
//ssize_t readAll(int fd, vector<char>& readTo){
//    readTo.clear();
//    char buf[1025];
//    ssize_t sum=0, once=0;
//    while(true) {
//        once = read(fd, buf, 1024);
//        if (once == -1) {
//            if (errno == EINTR)
//                continue;
//            else if (errno == EAGAIN)
//                return sum;
////            else if(errno == )
//        }
//        else if (once == 0)
//            return sum;
//        else {
//            sum+=once;
//            buf[once] = 0;
//            for(unsigned long i=0; i<once; i++)
//                readTo.push_back(buf[i]);
//        }
//    }
//}

int setSocketNonBlocking(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) return -1;
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1) return -1;
    return 0;
}

ssize_t writeAll(int fd, const string& writeFrom){
    char buf[4096];
    ssize_t sum=0, once=0;
    for(int i=0; i<writeFrom.size();) {
        int j = 0;
        while (j + i < writeFrom.size() && j < BUFFER_SZ) {
            buf[j] = writeFrom[i + j];
            j++;
        }
        buf[i + j] = 0;
        i+=j;
        once = write(fd, buf, j);
        if (once == -1){
            std::cout<< strerror(errno)<<std::endl;
            return -1;
        }
        else if (once == 0)
            return sum;
        else {
            sum += once;
        }
    }
    return sum;
}

int createFdThenBindListen(int port, int backlog){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
        return -1;
    int optval=1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval)) == -1) {
        close(fd);
        return -1;
    }

    setSocketNonBlocking(fd);

    sockaddr_in tobind{};
    memset(&tobind, 0, sizeof(tobind));
    tobind.sin_addr.s_addr = htonl(INADDR_ANY);
    tobind.sin_port =htons(port);
    tobind.sin_family = AF_INET;

    if( bind(fd, (sockaddr*)&tobind, sizeof(tobind))==-1){
        close(fd);
        return -1;
    }

    if( listen(fd, backlog) == -1){
        close(fd);
        return -1;
    }
    return fd;
}
