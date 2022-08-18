//
// Created by Clouver on 2022/8/15.
//



#include "reactor/Server.h"

using namespace std;

Server* serverForSig;

void handleSigInt(int sig){
    if (sig == SIGINT){
        cout<<"Exit"<<endl;
        serverForSig->getLoop()->stop();
        _exit(0);
    }
}

int main(void){
    Server server(8088, 2, 100);
    serverForSig = &server;
    signal(SIGINT, [](int sig){
        return handleSigInt(sig);
    }); // SIGINT 信号由 InterruptKey 产生，通常是 CTRL +C 或者 DELETE

    server.start();
}