//
// Created by Clouver on 2022/8/15.
//



#include "reactor/Server.h"
#include "reactor/http/HttpService.h"
#include "reactor/EventLoop.h"

#include <csignal>
#include <iostream>

using namespace std;

shared_ptr<Server> serverForSig;

void handleSigInt(int sig){
    if (sig == SIGINT){
        cout<<"Exit"<<endl;
        serverForSig->getLoop()->stop();
//        serverForSig.reset();
//        _exit(0);
    }
}

int main(int argc, char**argv){
    if(argc != 4)
        return 0;
    int port = atoi(argv[1]);
    int tnum = atoi(argv[2]);
    int conns = atoi(argv[3]);

    serverForSig = make_shared<Server>(port,tnum,conns,make_shared<HttpServiceFactory>());
    signal(SIGINT, [](int sig){
        return handleSigInt(sig);
    }); // SIGINT 信号由 InterruptKey 产生，通常是 CTRL +C 或者 DELETE
    serverForSig->start();
}