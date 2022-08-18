cmake_minimum_required(VERSION 3.22)
project(ToyWebserver)
find_package( OpenCV REQUIRED )

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES reactor/Server.cpp reactor/EventLoop.cpp reactor/Poller.cpp reactor/TcpConnection.cpp reactor/Channel.cpp reactor/tools/network.cpp
        reactor/http/HttpService.cpp reactor/http/HttpService.h
        reactor/http/tools.cpp reactor/http/tools.h
        reactor/img/ImageTrans.cpp reactor/img/ImageTrans.h)

add_executable(ToyWebserver ${SOURCE_FILES} main1.cpp)
target_link_libraries( ToyWebserver ${OpenCV_LIBS})