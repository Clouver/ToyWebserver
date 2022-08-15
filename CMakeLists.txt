cmake_minimum_required(VERSION 3.22)
project(ToyWebserver)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES reactor/Server.cpp reactor/EventLoop.cpp reactor/Poller.cpp reactor/TcpConnection.cpp reactor/Channel.cpp reactor/tools/network.cpp)
add_executable(ToyWebserver ${SOURCE_FILES} main1.cpp)
