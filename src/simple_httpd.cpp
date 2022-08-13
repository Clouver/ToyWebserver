

#include "simple_httpd.h"


int SERVER_MAX_CONN = 100;
bool ECHO_REQUEST_INFO = false;
bool ECHO_HEADERS = false;

// To read the HTTP Request.
// Presume length of request is limited which is a lazy way to implement.
int READ_BUFFER_SIZE = 1024;
string RES_DIR = "./res/";
string default_file = "404.html";


void RequestHeaders::parse(char* buf, int bufEnd){
/*
    cout<<"all content of request"<<endl;
    for(int i=0; i<bufEnd; i++)
        cout<<buf[i];
    cout<<endl;
*/
    const int TMP_SZ = 8192;
    int idx=0;
    char tmp[TMP_SZ], name[128];
    // method
    skipSpace(buf, idx, bufEnd);
    readString(tmp, ' ', TMP_SZ-1, buf, idx, bufEnd);
    method = tmp;

    // target resource
    skipSpace(buf, idx, bufEnd);
    readString(tmp, ' ', TMP_SZ-1, buf, idx, bufEnd);
    target = tmp;
    // default target resource should be "/index.html"
    if(target[target.size()-1] == '/' )
        target+="index.html";
    if(target[0] == '/')
        target = target.substr(1);

    // ignore " HTTP/"
    idx+=6;

    // main version number
    skipSpace(buf, idx, bufEnd);
    readString(tmp, '.', 8191, buf, idx, bufEnd);
    main_ver = atoi(tmp);

    idx++; // skip '.'
    // minor version number
    skipSpace(buf, idx, bufEnd);
    readString(tmp, '\n', 8191, buf, idx, bufEnd);
    mino_ver = atoi(tmp);

    // headers
    while(1){
        if(idx+1>=bufEnd || (buf[idx]=='\n'&&buf[idx+1]==13) )
            break;
        // Key
        skipSpace(buf, idx, bufEnd);
        readString(name, ':', 127, buf, idx, bufEnd);

        // skip ':'
        idx++;

        // Value
        skipSpace(buf, idx, bufEnd);
        readString(tmp, '\n', 8191, buf, idx, bufEnd);
        headers.insert( pair<string,string>(string(name), string(tmp)) );
    }


    if ( ECHO_HEADERS ){
        // ECHO
        // Chinese
        cout<<"HTTP version:    "<<main_ver<<'.'<<mino_ver<<endl;
        cout<<"Request Method:  "<<method<<endl;
        cout<<"Target Resource: "<<target<<endl;
        cout<<"Headers:         "<<endl;

        for(map<string,string>::iterator it=headers.begin(); it!=headers.end();it++){
            cout<<"                 "<<it->first.substr(0,15);
            if(it->first.size() > 15 )
                cout<<"...";
            else
                for(int i=0; i<15-(it->first.size())+3; i++)
                    cout<<' ';
            cout<<"\t:\t"<<it->second.substr(0, 20);

            if(it->second.size() > 20)
                cout<<"...";

            cout<<endl;

            if(it->first == "Accept")
                cout<<"                 "<<"客户端接收文件类型"<<endl;
            else if( it->first == "Accept-Encoding")
                cout<<"                 "<<"编码类型"<<endl;
            else if( it->first == "Accept-Language")
                cout<<"                 "<<"语言"<<endl;
            else if( it->first == "Cache-Control")
                cout<<"                 "<<"缓存控制"<<endl;
            else if( it->first == "Connection")
                cout<<"                 "<<"是否保持连接"<<endl;
            else if( it->first == "Host")
                cout<<"                 "<<"主机"<<endl;
            else if( it->first == "User-Agent")
                cout<<"                 "<<"用户代理(浏览器)"<<endl;
            else if( it->first == "Referer")
                cout<<"                 "<<"提供者"<<endl;
            else
                cout<<"                 "<<"..."<<endl;

        }
    }

}

ResponseHeaders::ResponseHeaders(){
    sz=0;
    bufsz = 4096;
    buf = new char[bufsz];
}

ResponseHeaders::~ResponseHeaders(){
    delete[] buf;
}

void ResponseHeaders::addFirstLine(string num, string tips){
    string ver = "HTTP/1.0";
    for(int i=0; i<ver.size(); i++,sz++)
        buf[sz]=ver[i];
    buf[sz++] = ' ';
    for(int i=0; i<num.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = num[i];
    buf[sz++] = ' ';
    for(int i=0; i<tips.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = tips[i];
    buf[sz++] = '\n';
}

void ResponseHeaders::addHeader(string key, string value){
    for(int i=0; i<key.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = key[i];
    buf[sz++] = ':';
    buf[sz++] = ' ';

    for(int i=0; i<value.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = value[i];
    buf[sz++] = '\n';
}

int ResponseHeaders::Write(int sk){
    buf[sz++] = '\n';
    buf[sz] = 0;
    return send(sk, buf, sz, MSG_NOSIGNAL);
}


// Send file "s" in "RES_DIR" to "sk". The file must be in "files" or default_file.
void SendFile(int sk, string s){

    // File name
    s = RES_DIR+s;

    // Open file & Get size
    FILE *fp = fopen(s.c_str(),"r");
    if(fp == NULL ){
        s= RES_DIR+default_file;
        fp = fopen(s.c_str(), "r");
        if(fp == NULL )
            return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /**************************************************************
     *                                                            *
     *  Here is a point for further development:                  *
     *  The ResponseHeaders can be obtained from other function.  *
     *                                                            *
     *  rh = GetResponseHeaders( string fileName, etc. );         *
     *                                                            *
     **************************************************************/
    // Make response headers & Write headers
    ResponseHeaders rh;

    if(s == RES_DIR+default_file)
        rh.addFirstLine("404", "Not Found");
    else
        rh.addFirstLine("200", "OK");

    rh.addHeader("Server", "Simple-Httpd");
    rh.addHeader("Connection", "close");
    rh.addHeader("Content-Type", getType(s));

    char sz_s[1024];
    sprintf(sz_s, "%ld", fileSize);
    rh.addHeader("Content-Length", sz_s);

    rh.Write(sk);

    // Write file
    char buffer[256];
    while(!feof(fp)){
        int readed = fread(buffer, sizeof(char), 256, fp);
        send(sk, buffer, readed, MSG_NOSIGNAL);
    }
    fclose(fp);
}

// preRead length max == READ_BUFFER_SIZE
void SolveRequest(int sk, const char* preRead){
    // set SO_LINGER for close socket

    struct linger so_linger;
    so_linger.l_onoff = true;
    so_linger.l_linger = 10;
    setsockopt(sk,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));

    // Read request
    char buf[READ_BUFFER_SIZE];
    if (preRead != nullptr)
        memcpy(buf, preRead, READ_BUFFER_SIZE);
    else
        read(sk, buf, READ_BUFFER_SIZE-1);

    if( ECHO_REQUEST_INFO )
        cout<<buf<<endl;

    // Parsing headers
    RequestHeaders h;
    h.parse(buf, READ_BUFFER_SIZE-1);

    // Response
    SendFile(sk, h.target);



    // Close connection & Set thread flag
    //shutdown(sk, SHUT_RDWR);


    return;
}

HttpServer::HttpServer(){
    sockfd = portno = -1;
    MAX_CONNECT = SERVER_MAX_CONN;  // Max thread number
}
HttpServer::HttpServer(int port){
    sockfd = -1;
    SetPort(port);
    MAX_CONNECT = SERVER_MAX_CONN;  // Max thread number
}
void HttpServer::SetPort(int port){
    portno = port;
    bzero((char*)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portno);
}
void HttpServer::bindAndListen() {
    if(portno < 0 || portno > 65535)
        error("ERROR wrong portno");

    // Listen
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    int opt = SO_REUSEADDR;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if ( bind(sockfd, (sockaddr*)&server_address, sizeof(server_address)) < 0)
        error("ERROR binding");
    listen(sockfd, MAX_CONNECT);
}



IterativeHttpServer::IterativeHttpServer():HttpServer(){};
IterativeHttpServer::IterativeHttpServer(int port):HttpServer(port){};
void IterativeHttpServer::acceptAndServe(void){
    bindAndListen();
    int newsockfd = 0;
    struct sockaddr_in cli;
    socklen_t clilen  = sizeof(cli);

    while( 1 ){
        // Accept New Connection & Solve it
        newsockfd = accept(sockfd, (sockaddr*)&cli, &clilen);
        // ECHO
        if( ECHO_REQUEST_INFO ){
            cout<<"\n\nNew Request From: "<<inet_ntoa(cli.sin_addr)<<endl;
        }
        if (newsockfd < 0)
            error("ERROR on accept");
        else{
            SolveRequest(newsockfd, nullptr);
        }
    }

}
void IterativeHttpServer::setArg(int arg){};


MultiProcessedHttpServer::MultiProcessedHttpServer():HttpServer(){};
MultiProcessedHttpServer::MultiProcessedHttpServer(int port):HttpServer(port){};
void MultiProcessedHttpServer::acceptAndServe(void){
    bindAndListen();
    signal(SIGCHLD, SIG_IGN);

    int newsockfd = 0;
    struct sockaddr_in cli;
    socklen_t clilen  = sizeof(cli);
    while( 1 ){

        // Accept New Connection & Solve it
        newsockfd = accept(sockfd, (sockaddr*)&cli, &clilen);

        int pid = fork();
        if( pid < 0){
            perror("Error at creating process");
            exit(-1);
        }
        else if( pid > 0 ){
            close(newsockfd);
            continue;
        }
        else if( pid == 0){
            close(sockfd);
            // ECHO
            if( ECHO_REQUEST_INFO ){
                cout<<"\n\nNew Request From: "<<inet_ntoa(cli.sin_addr)<<endl;
            }
            if (newsockfd < 0)
                error("ERROR on accept");
            else{
                SolveRequest(newsockfd, nullptr);
            }
            _exit(0);
        }
    }
}
void MultiProcessedHttpServer::setArg(int arg){};

//MultiThreadedHttpServer::MultiThreadedHttpServer():HttpServer() {};
//MultiThreadedHttpServer::MultiThreadedHttpServer(int port):HttpServer(port) {};
//void MultiThreadedHttpServer::acceptAndServe(void) {
//
//    bindAndListen();
//    int newsockfd = 0;
//    struct sockaddr_in cli;
//    socklen_t clilen = sizeof(cli);
//
//    while (1) {
//        // Accept New Connection & Solve it
//        newsockfd = accept(sockfd, (sockaddr *) &cli, &clilen);
//        if (newsockfd < 0)
//            perror("ERROR on accept");
//        else
//        {
//            auto routineForPThread = [](void* arg)->void* {
//                pthread_detach(pthread_self());
//                // use "add_compile_options(-fpermissive)" in cmakelist to avoid ERROR
//                int fd = int(arg);
//                SolveRequest( fd, nullptr );
//                pthread_exit(NULL);
//            };
//            // ECHO
//            if (ECHO_REQUEST_INFO) {
//                cout << "\n\nNew Request From: " << inet_ntoa(cli.sin_addr) << endl;
//            }
//            pthread_t pid;
//            if (   0 != pthread_create( &pid, nullptr, routineForPThread, (void*)newsockfd) ){
//                perror("Error at creating thread");
//            }
//        }
//    }
//}
//void MultiThreadedHttpServer::setArg(int arg){};

IOMultiplexingHttpServer::IOMultiplexingHttpServer():HttpServer(){};
IOMultiplexingHttpServer::IOMultiplexingHttpServer(int port):HttpServer(port){};
void IOMultiplexingHttpServer::acceptAndServe() {
    bindAndListen();

    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);
    for (int i =0; i< FD_SETSIZE; i++)
        fds[i] = -1;

    int maxfd = sockfd;
    int maxi = -1;
    struct sockaddr_in cli;
    socklen_t clilen = sizeof(cli);
    while (1) {
        rset = allset;
        int nready = select (maxfd+1, &rset, NULL, NULL, NULL);
        int connectfd;
        if (FD_ISSET(sockfd, &rset)) {
            if ((connectfd = accept( sockfd, (struct sockaddr *)&cli, &clilen)) == -1)  {
                perror("accept error.");
                continue;
            }
            int i;
            for (i = 0; i < FD_SETSIZE; i++)
                if (fds[i] < 0) {
                    fds[i] = connectfd;
                    break;
                }
            if (i == FD_SETSIZE)    printf("too many cllients.\n");
            FD_SET(connectfd, &allset);
            if (connectfd > maxfd)   maxfd = connectfd;
            if (i > maxi)   maxi =i;
            if ( -- nready <=0) continue;
        }  /* if (FD_ISSET (listenfd… */
        int readfd;
        char preRead[READ_BUFFER_SIZE];
        for (int i = 0; i <= maxi; i++)  {
            if ((readfd = fds[i]) < 0)    continue;
            if (FD_ISSET(readfd, &rset)) {
                if( read(readfd, preRead, READ_BUFFER_SIZE-1) >0 ){
                    SolveRequest(readfd, preRead );
                }
                close(readfd);
                FD_CLR(readfd, &allset);
                fds[i] = -1;
                if ( --nready <=0 )  break;
            }  /* if (FD_ISSET(sockfd, &rset))  */
        } /* for(i = 0; i <= maxi; i++)   */
    }  /* while(1); */

    close(sockfd);
}
void IOMultiplexingHttpServer::setArg(int arg){};

//pthread_cond_t ProcessPooledHttpServer::cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t* ProcessPooledHttpServer::pMutex = nullptr;
void ProcessPooledHttpServer::setProcessNum(int num) {
    processNum = num;
}
void ProcessPooledHttpServer::setArg(int arg){
    setProcessNum(arg);
}
ProcessPooledHttpServer::ProcessPooledHttpServer(int port):HttpServer(port){
    setProcessNum(10);
    initMutex();
}
void ProcessPooledHttpServer::mutexLock(){
    pthread_mutex_lock(pMutex);
}
void ProcessPooledHttpServer::mutexRelease(){
    pthread_mutex_unlock(pMutex);
}

int ProcessPooledHttpServer::initMutex(){
    int	fd;
    pthread_mutexattr_t	mattr;
    fd = open("/dev/zero", O_RDWR, 0);
    pMutex = (pthread_mutex_t*)mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,	MAP_SHARED, fd, 0);
    close(fd);
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(pMutex, &mattr);

    /*
    pthread_condattr_t condattr;
    pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);

    pthread_condattr_init(&condattr);
    pthread_cond_init(&cond,&condattr);
    */
}

void ProcessPooledHttpServer::acceptAndServe() {
    bindAndListen();
    for(int i=0; i<processNum; i++){
        int pid = fork();
        if(pid==0){//  child process start

            while(1){
                int newsockfd = 0;
                struct sockaddr_in cli;
                socklen_t clilen = sizeof(cli);
                // Accept New Connection & Solve it
                mutexLock();
                newsockfd = accept(sockfd, (sockaddr *) &cli, &clilen);
                mutexRelease();

                SolveRequest(newsockfd, nullptr);
            }
            _exit(0);
        }// child process end; exit
        else{
            continue; // main process continue;
        }
    }// end creating child processes.
    sleep(-1);
}




//pthread_mutex_t ThreadPooledHttpServer::mutex = PTHREAD_MUTEX_INITIALIZER;
//void* ThreadPooledHttpServer::threadFunc(void* arg){
//    int _sockfd = (int)arg;
//    while(1){
//        int newsockfd = 0;
//        struct sockaddr_in cli;
//        socklen_t clilen = sizeof(cli);
//        // Accept New Connection & Solve it
//        mutexLock();
//        newsockfd = accept(_sockfd, (sockaddr *) &cli, &clilen);
//        mutexRelease();
//        SolveRequest(newsockfd, nullptr);
//    }
//}
//void ThreadPooledHttpServer::makeThread() {
//    pthreads = new pthread_t[threadNum];
//    for(int i=0; i<threadNum; i++)
//        pthread_create( &pthreads[i], nullptr,
//                        threadFunc, (void*)sockfd );
//
//}
//ThreadPooledHttpServer::ThreadPooledHttpServer():HttpServer(){};
//ThreadPooledHttpServer::ThreadPooledHttpServer(int port):HttpServer(port){
//    setThreadNum(10);
//    initMutex();
//};
//void ThreadPooledHttpServer::setThreadNum(int num){
//    threadNum = num;
//};
//void ThreadPooledHttpServer::setArg(int arg){
//    setThreadNum(arg);
//}
//void ThreadPooledHttpServer::acceptAndServe(void) {
//    bindAndListen();
//
//    makeThread();
//
//    sleep(-1);
//};
//int ThreadPooledHttpServer::initMutex(void){
//    mutex = PTHREAD_MUTEX_INITIALIZER;
//};
//void ThreadPooledHttpServer::mutexLock(){
//    pthread_mutex_lock(&mutex);
//};
//void ThreadPooledHttpServer::mutexRelease(){
//    pthread_mutex_unlock(&mutex);
//};


PipelinedHttpServer::toSend::toSend() {};
PipelinedHttpServer::toSend::toSend(int sk, string target) {
    this->sk = sk;
    this->target = target;
};

pthread_mutex_t PipelinedHttpServer::valve1Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PipelinedHttpServer::valve2Mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t PipelinedHttpServer::valve1Sem = sem_t();
sem_t PipelinedHttpServer::valve2Sem = sem_t();
queue<int> PipelinedHttpServer::toHeaderProcessQueue = queue<int>();
queue<PipelinedHttpServer::toSend> PipelinedHttpServer::toSendQueue = queue<PipelinedHttpServer::toSend>();
PipelinedHttpServer::PipelinedHttpServer():HttpServer() { numOfValve1 = 10; numOfValve2 = 10;};
PipelinedHttpServer::PipelinedHttpServer(int port):HttpServer(port) {numOfValve1 = 10; numOfValve2 = 10;};
void PipelinedHttpServer::setArg(int arg) { numOfValve1 = numOfValve2 = arg; }
void* PipelinedHttpServer::headerProcessValve(void* arg){
    pthread_detach(pthread_self());

    // set SO_LINGER for close socket
    struct linger so_linger;
    so_linger.l_onoff = true;
    so_linger.l_linger = 10;

    while(1){
        int sk=-1;
        // to improve with cond!
        sem_wait(&valve1Sem);
        pthread_mutex_lock(&valve1Mutex);
        //pthread_cond_wait(&valve1cond, &valve1Mutex);
        if( !toHeaderProcessQueue.empty() ){
            sk = toHeaderProcessQueue.front();
            toHeaderProcessQueue.pop();
        }
        pthread_mutex_unlock(&valve1Mutex);
        if(sk>0){
            setsockopt(sk,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
            char buf[READ_BUFFER_SIZE];
            read(sk, buf, READ_BUFFER_SIZE-1);
            if( ECHO_REQUEST_INFO )
                cout<<buf<<endl;
            // Parsing headers
            RequestHeaders h;
            h.parse(buf, READ_BUFFER_SIZE-1);
            // to improve with cond!
            pthread_mutex_lock(&valve2Mutex);
            toSendQueue.push(toSend(sk, h.target));
            sem_post(&valve2Sem);
            pthread_mutex_unlock(&valve2Mutex);
        }
    }
}
void* PipelinedHttpServer::sendingValve(void* arg){
    pthread_detach(pthread_self());
    toSend tos;
    while(1){
        int flag=0;
        sem_wait(&valve2Sem);
        pthread_mutex_lock(&valve2Mutex);
        if(!toSendQueue.empty()){
            tos = toSendQueue.front();
            toSendQueue.pop();
            flag=1;
        }
        pthread_mutex_unlock(&valve2Mutex);
        if(flag==1){
            SendFile(tos.sk, tos.target);
            close(tos.sk);
        }
    }
}

void PipelinedHttpServer::acceptAndServe() {
    sem_init(&valve1Sem,0,0);
    sem_init(&valve2Sem,0,0);
    bindAndListen();

    pValve1 = new pthread_t[numOfValve1];
    for(int i=0; i<numOfValve1; i++){
        for(int i=0; i<numOfValve1; i++)
            pthread_create( &pValve1[i], nullptr, headerProcessValve, nullptr );
    }
    pValve2 = new pthread_t[numOfValve2];
    for(int i=0; i<numOfValve2; i++){
        for(int i=0; i<numOfValve2; i++)
            pthread_create( &pValve2[i], nullptr, sendingValve, nullptr );
    }

    while(1){
        int newsockfd = 0;
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        // Accept New Connection & Solve it
        newsockfd = accept( sockfd, (sockaddr *) &cli, &clilen);
        if( newsockfd > 0){
            pthread_mutex_lock(&valve1Mutex);
            toHeaderProcessQueue.push(newsockfd);
            sem_post(&valve1Sem);
            pthread_mutex_unlock(&valve1Mutex);
        }
    }

}




/*
HttpClient::HttpClient(): sockfd(-1), portno(-1), phost(nullptr){}

HttpClient::HttpClient(string hostName, int port): phost(nullptr){
    SetHost(hostName, port);
}

void HttpClient::SetHost(string hostName, int port){
    this->hostName = hostName;
    portno = port;
}

void HttpClient::DNS_Parse(void){
    phost = gethostbyname(hostName.c_str());
    if (phost == NULL){
        fprintf(stderr,"ERROR Failed at DNS parsing");
        exit(0);
    }
}

void HttpClient::Prepare(void){
    DNS_Parse();
    bzero((char*)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char*)phost->h_addr, (char*)&server_address.sin_addr.s_addr, sizeof(phost->h_addr));
    server_address.sin_port = htons(portno);
}

void HttpClient::Connect(void){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        error("Error Opening Socket");
    if( connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0 )
        error("Error Connecting");
}

void testThread(void){
    HttpClient client("localhost", 8080);
    client.Prepare();
    client.Connect();
}
*/
