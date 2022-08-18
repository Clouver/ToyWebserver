//
// Created by Clouver on 2022/8/14.
//

#include "Server.h"

const int connBacklog = 3000;

Server::Server():Server(0,0,connBacklog){};

shared_ptr<EventLoop> Server::getLoop() const {
    return mainLoop;
}

Server::Server(int port, int threadNum, int maxConnSize):fd_(0), port_(port),
                                                         threadNum_(threadNum),maxConnSize_(maxConnSize){
    /*
     * Server 由几个部分组成：
     *      mainloop                即主reactor。这里不单开线程了，直接主线程进行loop。 也没有单独的 Acceptor，mainloop就是acceptor，只修改一下handleread。
     *      subLoops + threads      即从reactors。线程池。
     *      connOfFd                保存已建立的连接。（包含了acceptChannel所在的伪连接）
     *      connManager             负责释放已关闭的连接。
     */

    setMainLoop();
    runSubreactors();
    runConnManager();
}

void Server::runSubreactors(){
    for(int i=0; i<threadNum_; i++){
        subLoops.emplace_back(new EventLoop());
        subLoops.back()->start(); // 必须先设置 start，再创建线程。
        threads.push_back( make_shared<std::thread>(bind(&EventLoop::loop, subLoops.back())) );
    }
}

void Server::setMainLoop(){
    fd_ = createBindListen(port_, maxConnSize_);

    if(fd_ == -1)
        throw exception();

    /*
     * AcceptChannel 的定义。
     * 传入server指针。
     * 放入mainloop。
     *
     * acceptchannel 的 owner 为空
     */
    mainLoop = make_shared<EventLoop>();
    ch_ = make_shared<Channel>(fd_);
    ch_->setReadCallback( bind(&Server::handleConn, this));
    mainLoop->poller->add(ch_);
}


void Server::runConnManager(){
    connManaging = true;
    connectionManager = make_shared<std::thread>( bind(&Server::destroyClosedConn, this));
}

void Server::destroyClosedConn(){
    while(connManaging){
        unique_lock<mutex> lock(connMutex);
        while(toClose.empty())
            connCond.wait(lock);

        while(!toClose.empty()){
            int fd = toClose.front();
            toClose.pop();
            shared_ptr<TcpConnection> conn = connOfFd[fd];
            connOfFd.erase(fd); // 析构即释放资源
            conn->release();
        }
        lock.unlock();
    }
}

// 提醒 manager
void Server::handleClose(int fd){
    unique_lock<mutex>lock(connMutex);
    toClose.push(fd);
    lock.unlock();
    connCond.notify_one();
}

void Server::handleConn(){
    while(true){
        sockaddr addr{};
        socklen_t len = sizeof(addr);
        int newFd = accept(fd_, &addr, &len);
        if(newFd <= 0)
            break;

        setSocketNonBlocking(newFd);

        if(newFd == -1)
            return;

        shared_ptr<EventLoop>&loop = subLoops[rand()%threadNum_];
        shared_ptr<TcpConnection> conn = make_shared<TcpConnection>(newFd, loop);
        conn->getChannel()->setOwner(conn);
        conn->setCloseCallback( bind(&Server::handleClose, this, newFd));
        connOfFd[newFd] = conn;
    }
}

void Server::start(){
    mainLoop->start();
    mainLoop->loop();
}

Server::~Server(){
    for(int i=0; i<threadNum_; i++)
        subLoops[i]->stop();
}