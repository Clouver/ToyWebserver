//
// Created by Clouver on 2022/8/28.
//

#ifndef TOYWEBSERVER_SERVICE_H
#define TOYWEBSERVER_SERVICE_H

#include <vector>
#include <memory>
#include "tools/Buffer.h"

class Service {
public:
    virtual void SolveRequest(int sk, Buffer &buf)=0;
    virtual ~Service() = default;
};

class ServiceFactory{
public:
    virtual void create(std::shared_ptr<Service>& spService)=0;
};



#endif //TOYWEBSERVER_SERVICE_H
