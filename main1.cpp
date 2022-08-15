//
// Created by Clouver on 2022/8/15.
//



#include "reactor/Server.h"

using namespace std;

int main(void){
    Server server(8088, 1, 100);
    server.start();
}