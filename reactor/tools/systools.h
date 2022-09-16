//
// Created by Clouver on 2022/9/16.
//

#ifndef TOYWEBSERVER_SYSTOOLS_H
#define TOYWEBSERVER_SYSTOOLS_H

#include <sys/resource.h>
#include <cerrno>
#include <iostream>

int setNOFILE(int n){
    rlimit64 lm{};
    getrlimit64(RLIMIT_NOFILE, &lm);
    lm.rlim_max = n;
    lm.rlim_cur = n;
    if (setrlimit64(RLIMIT_NOFILE, &lm) == -1){
        if(errno == EPERM){
            std::cout<<"Run in previlege for higher FDs num limit"<<std::endl;
        }
        else{
            std::cout<<"?"<<std::endl;
        }
        return -1;
    }
    return 0;
}

#endif //TOYWEBSERVER_SYSTOOLS_H
