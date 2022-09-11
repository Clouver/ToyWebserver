//
// Created by Clouver on 2022/8/28.
//

#include "../PingPongService.h"

#include "../../../reactor/Server.h"

#include <memory>

using namespace std;

class Server;
class HttpServiceFactory;

static int port = 8888;
static int tnum = 1;
static int conns = 1024;

using namespace std;

int main(int argc, char**argv){
    if(argc != 3){
        cout<<"usage: "<<argv[0]<<" maxThread maxConn ."<<endl;
        return 0;
    }
//    port = atoi(argv[1]);
    tnum = atoi(argv[1]);
    conns = atoi(argv[2]);
    shared_ptr<Server> server = make_shared<Server>(port,
                                                    tnum,
                                                    conns,
                                                    make_shared<PingPongServiceFactory>());
    server->start();
}

