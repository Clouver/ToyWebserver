#pragma once

#include<semaphore.h>
#include <csignal>
#include <fcntl.h>
#include <sys/mman.h>
#include <thread>
#include <pthread.h>
// for process pool & thread pool
#include <mutex>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// for socket
#include <netdb.h>


#include <queue>
// for hostent in HttpClient
#include <unistd.h>
// for write() \ read()
#include <strings.h>
// for bzero()
#include <map>
#include <string>
#include <iostream>
#include <string.h>

#include "tools.h"
using namespace std;


//void* routineForPThread( void* arg);

class RequestHeaders{

public:
    map<string,string>headers;
    int main_ver, mino_ver;
    string method, target;

    void parse(char* buf, int bufEnd);
};

class ResponseHeaders{

    int sz;
    int bufsz;
    char* buf;
public:
    ResponseHeaders();
    ~ResponseHeaders();

    void addFirstLine(string num, string tips);

    void addHeader(string key, string value);

    int Write(int sk);
};

// Send file "s" in "RES_DIR" to "sk". The file must be in "files" or default_file.
void SendFile(int sk, string s);

void SolveRequest(int sk, const char* preRead );

class HttpServer{
protected:
    int portno;
    int sockfd;
    sockaddr_in server_address;

    int MAX_CONNECT;
public:
    HttpServer();
    HttpServer(int port);
    void SetPort(int port);
    void bindAndListen(void);
    virtual void acceptAndServe(void)=0;
    virtual void setArg(int arg)=0;
};

class IterativeHttpServer : public HttpServer{
public:
    IterativeHttpServer();
    IterativeHttpServer(int port);
    // Override
    void acceptAndServe(void);
    void setArg(int arg);
};

class MultiProcessedHttpServer : public HttpServer{
public:
    // Override
    MultiProcessedHttpServer();
    MultiProcessedHttpServer(int port);
    void acceptAndServe(void);
    void setArg(int arg);
};

class MultiThreadedHttpServer : public HttpServer{
public:
    // Override
    MultiThreadedHttpServer();
    MultiThreadedHttpServer(int port);
    void acceptAndServe(void) ;
    void setArg(int arg);
};

class IOMultiplexingHttpServer : public HttpServer{
private:
    int fds[FD_SETSIZE];
    fd_set rset, allset;
public:
    // Override
    IOMultiplexingHttpServer();
    IOMultiplexingHttpServer(int port);
    void acceptAndServe(void) ;
    void setArg(int arg);
};

class ProcessPooledHttpServer : public HttpServer{
private:
    static pthread_mutex_t *pMutex;
    //static pthread_cond_t cond;
    //queue<int> requests;
    int processNum;
    void setProcessNum(int num);
    int initMutex(void);
    static void mutexLock();
    static void mutexRelease();
public:
    // Override
    ProcessPooledHttpServer();
    ProcessPooledHttpServer(int port);
    void acceptAndServe(void) ;
    void setArg(int arg);
};


class ThreadPooledHttpServer : public HttpServer{
private:
    static pthread_mutex_t mutex;
    pthread_t *pthreads;
    int threadNum;
    static void* threadFunc(void* arg);
    int initMutex(void);
    void makeThread();
    void setThreadNum(int num);
    static void mutexLock();
    static void mutexRelease();
public:
    ThreadPooledHttpServer();
    ThreadPooledHttpServer(int port);
    void acceptAndServe(void) ;
    void setArg(int arg);
};

class PipelinedHttpServer : public HttpServer{
private:
    class toSend{
    public:
        int sk;
        string target;
        toSend();
        toSend(int sk, string target);
    };
    pthread_t *pValve1;
    pthread_t *pValve2;
    int numOfValve1;
    int numOfValve2;
    static sem_t valve1Sem;
    static sem_t valve2Sem;
    static pthread_mutex_t valve1Mutex;
    static pthread_mutex_t valve2Mutex;

    static queue<int> toHeaderProcessQueue;
    static queue<toSend> toSendQueue;
    static void* headerProcessValve(void* arg);
    static void* sendingValve(void* arg);
public:
    PipelinedHttpServer();
    PipelinedHttpServer(int port);
    void acceptAndServe(void) ;
    void setArg(int arg);
};


// Designed for testing HttpServer
/*
class HttpClient{

std::string hostName;

sockaddr_in server_address;

int sockfd, portno;

hostent* phost;

char buffer[256];

public:
    HttpClient();
    HttpClient(string hostName, int port);

    void SetHost(string hostName, int port);

    void DNS_Parse(void);

    void Prepare(void);

    void Connect(void);
};


void testThread(void);
*/
