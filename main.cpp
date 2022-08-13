
#include <csignal>
#include "src/simple_httpd.h"

using namespace std;

extern int SERVER_MAX_CONN;
extern bool ECHO_REQUEST_INFO;
extern bool ECHO_HEADERS;
extern string RES_DIR;
extern string default_file;

extern int READ_BUFFER_SIZE;

int main(int argc, char* argv[]){
    //signal(SIGPIPE, SIG_IGN);

    if(argc < 4 || argc >5){
        cout<<"usage:\t"<<argv[0]<<" port server_type arg [res-dir]"<<endl;
        cout<<"         port:           http listen port"<<endl;
        cout<<"         server_type:    1.iterative"<<endl;
        cout<<"                         2.multi-process"<<endl;
        cout<<"                         3.multi-thread"<<endl;
        cout<<"                         4.IO Multiplex"<<endl;
        cout<<"                         5.process pool"<<endl;
        cout<<"                         6.thread pool"<<endl;
        cout<<"                         7.pipeline"<<endl;
        cout<<"         arg(type n):    1.Nope"<<endl;
        cout<<"                         2.Nope"<<endl;
        cout<<"                         3.Nope"<<endl;
        cout<<"                         4.Nope"<<endl;
        cout<<"                         5.Process Pool Size"<<endl;
        cout<<"                         6.Thread Pool Size"<<endl;
        cout<<"                         7.Number of Valve2"<<endl;
        return 0;
    }
    int port = atoi(argv[1]);
    int type = atoi(argv[2]);
    int arg = atoi(argv[3]);
    if(argc == 5)
        RES_DIR = string(argv[4]);
    if(RES_DIR[RES_DIR.size()-1] != '/')
        RES_DIR+="/";

    HttpServer *server;
    switch(type){
        case 1: server = new IterativeHttpServer(port);break;
        case 2: server = new MultiProcessedHttpServer(port);break;
//        case 3: server = new MultiThreadedHttpServer(port);break;
        case 4: server = new IOMultiplexingHttpServer(port);break;
        case 5: server = new ProcessPooledHttpServer(port); server->setArg(arg);break;
//        case 6: server = new ThreadPooledHttpServer(port); server->setArg(arg);break;
        case 7: server = new PipelinedHttpServer(port); server->setArg(arg);break;
            //case 7: server = new PipelinedHttpServer(port);break;
        default:
            cout<<"wrong server type number!"<<endl;
            return 0;
    }
    //IterativeHttpServer server( atoi(argv[1]) );
    //MultiProcessedHttpServer server( atoi(argv[1]));
    //MultiThreadedHttpServer server( atoi(argv[1]));
    //IOMultiplexingHttpServer server( atoi(argv[1]));

    //ProcessPooledHttpServer server( atoi(argv[1]));
    //server.setProcessNum(10);

    server->acceptAndServe();

}

