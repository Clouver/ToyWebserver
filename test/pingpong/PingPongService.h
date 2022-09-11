//
// Created by Clouver on 2022/9/9.
//

#ifndef TOYWEBSERVER_PINGPONGSERVICE_H
#define TOYWEBSERVER_PINGPONGSERVICE_H

#include <mutex>
#include "../../reactor/Service.h"
#include "../../reactor/tools/Timer.h"

class PingPongService : public Service {
public:
    static double sumCost;
    static size_t sumCnt;
    static size_t sumSize;
    static mutex m;

    double cost=0;
    size_t cnt = 0;
    size_t size = 0;

    static const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE]{};

    void SolveRequest(int sk, Buffer &buf) override;

    ~PingPongService() override;
};

class PingPongServiceFactory : public ServiceFactory{
public:
    void create(std::shared_ptr<Service>& spService) override;
};

#endif //TOYWEBSERVER_PINGPONGSERVICE_H
