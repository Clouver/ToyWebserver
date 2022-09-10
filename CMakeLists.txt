cmake_minimum_required(VERSION 3.16)
project(ToyWebserver)


find_package( OpenCV REQUIRED )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(test/pingpong/ppserver)
add_subdirectory(test/pingpong/ppclient)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

set(SOURCE_FILES reactor/Server.cpp
        reactor/EventLoop.cpp
        reactor/Poller.cpp
        reactor/TcpConnection.cpp
        reactor/Channel.cpp
        reactor/tools/network.cpp
        reactor/http/HttpService.cpp
        reactor/http/tools.cpp
        reactor/img/ImageTrans.cpp
        reactor/Service.cpp reactor/Service.h
        reactor/tools/Timer.cpp reactor/tools/Timer.h
        reactor/tools/CircleQueue.h reactor/tools/CircleQueue.cpp reactor/tools/Buffer.cpp reactor/tools/Buffer.h)

add_executable(ToyWebserver ${SOURCE_FILES} main.cpp)
target_link_libraries( ToyWebserver ${OpenCV_LIBS})

add_custom_command(TARGET ToyWebserver PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:ToyWebserver>/res)