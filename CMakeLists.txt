cmake_minimum_required(VERSION 3.22)
project(ToyWebserver)

find_package( OpenCV REQUIRED )


set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES reactor/Server.cpp
        reactor/EventLoop.cpp
        reactor/Poller.cpp
        reactor/TcpConnection.cpp
        reactor/Channel.cpp
        reactor/tools/network.cpp
        reactor/http/HttpService.cpp
        reactor/http/tools.cpp
        reactor/img/ImageTrans.cpp)

add_executable(ToyWebserver ${SOURCE_FILES} main.cpp)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/dst)

add_custom_command(TARGET ToyWebserver PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:ToyWebserver>/res)

target_link_libraries( ToyWebserver ${OpenCV_LIBS})