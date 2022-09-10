//
// Created by Clouver on 2022/8/28.
//

#include "../../../reactor/EventLoop.h"
#include "../../../reactor/Channel.h"
#include "../../../reactor/TcpConnection.h"
#include "../../../reactor/Server.h"
#include "../../../reactor/tools/network.h"
#include "../../../reactor/tools/Timer.h"
#include "../PingPongService.h"

#include <memory>
#include <sys/socket.h>
#include <netdb.h> // for hostent
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // for tcpnodelay

#include <thread>
#include <cstring>

using namespace std;

const int port = 8888;

int main(int argc, char**argv){

    vector<shared_ptr<EventLoop>>loops;
    vector<shared_ptr<TcpConnection>>conns;
    vector<thread>threads;
    if(argc != 3){
        cout<<"usage: "<<argv[0]<<"<numOfConnections> <duration>"<<endl;
        return 0;
    }
    int MAX_CONNS = atoi(argv[1]);
    int sleepTime = atoi(argv[2]);

    shared_ptr<Server> server = make_shared<Server>();
    shared_ptr<PingPongServiceFactory> connFact = make_shared<PingPongServiceFactory>();

    struct hostent *he = gethostbyname("127.0.0.1");

    sockaddr_in dst{};
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);
    dst.sin_addr = *((struct in_addr*)he->h_addr);

    vector<char>buf(4096, 'x');

    int t=1;
    for(int i=0; i<MAX_CONNS; i++){

        int newFd = socket(AF_INET, SOCK_STREAM, 0);

        if(connect(newFd, (sockaddr*)&dst, sizeof(dst)) != 0){
            cout<<"connect failed "<<endl;
            strerror(errno);
            continue;
        }
        t = 1;
        setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY,&t, sizeof t);
        setSocketNonBlocking(newFd);

        sockaddr_in addr{};
        shared_ptr<TcpConnection> conn = TcpConnectionFactory::create(
                newFd,
                reinterpret_cast<sockaddr_in*>(&addr),
                make_shared<Server>(),
                        connFact);

//        cout<<"wtf? epoll_create1 return 0?"<<endl;
        epoll_create1(EPOLL_CLOEXEC);  // todo 在connect 之后，第一次epoll_create 会异常返回0，但是又没有errno。

        conns.push_back(conn);
        loops.push_back(make_shared<EventLoop>());
        auto& loop = loops.back();
        loop->addChannel(conn->getChannel());
        conn->getChannel()->setCloseCallback(bind( &EventLoop::delChannel, loop, conn->getChannel()) );
    }

    Timer timer("PingPong");
    timer.tick();
    for(int i=0; i<loops.size(); i++){
        auto& ploop = loops[i];
        ploop->start();
        write(conns[i]->getChannel()->getfd(), &*buf.begin(), 4096);    // 发送第一次
        threads.emplace_back( thread( bind(&EventLoop::loop, ploop)) );
    }

    sleep(sleepTime);
    for(auto& ploop : loops)
        ploop->stop();
    for(auto& th : threads)
        th.join();

    conns.clear();
    loops.clear();

    double second = timer.tock();
    cout<< "time cost       :  "<<second <<endl;
    cout<< "recived pack num:  "<<PingPongService::sumCnt<<endl;
    cout<< "recived data size_: "<<PingPongService::sumSize<<endl;
    cout<< "IO               : "<<PingPongService::sumSize/second/1024/1024<<" MB/s"<<endl;


}
