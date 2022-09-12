//
// Created by Clouver on 2022/9/9.
//

#include "PingPongService.h"

double PingPongService::sumCost;
size_t PingPongService::sumCnt;
size_t PingPongService::sumSize;
mutex PingPongService::m;

void PingPongServiceFactory::create(std::shared_ptr<Service>& spService){
    spService = std::make_shared<PingPongService>();
}

int PingPongService::SolveRequest(int sk, Buffer &buf){
    static Timer timer("\t\twrite");
    timer.tick();
    write(sk, buf.buf(), buf.size());

    cost += timer.tock();
    size += buf.size();
    cnt++;
    return 0;
}

PingPongService::~PingPongService(){
//    cout<<::syscall(SYS_gettid)<<endl;
//    std::cout << "io cost sum: " << cost << std::endl;
//    std::cout << "io cost per message: " << cost / cnt << std::endl;

    lock_guard<mutex>lock(m);
    sumCnt += cnt;
    sumCost += cost;
    sumSize += size;
}
