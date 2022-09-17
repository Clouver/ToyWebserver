
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

set(SOURCE_FILES ../../../reactor/Server.cpp
        ../../../reactor/EventLoop.cpp
        ../../../reactor/Poller.cpp
        ../../../reactor/TcpConnection.cpp
        ../../../reactor/Channel.cpp
        ../../../reactor/tools/network.cpp
        ../../../reactor/http/HttpService.cpp
        ../../../reactor/http/tools.cpp
        ../../../reactor/img/ImageTrans.cpp
        ../../../reactor/Service.cpp
        #        ppserver.cpp
        ../../../reactor/tools/Timer.cpp ../../../reactor/tools/Timer.h
        ../PingPongService.cpp ../PingPongService.h
        ../../../reactor/tools/CircleQueue.h ../../../reactor/tools/CircleQueue.cpp ../../../reactor/tools/Buffer.cpp ../../../reactor/tools/Buffer.h ../../../reactor/ConnectionSet.cpp ../../../reactor/ConnectionSet.h
#        ../../../reactor/uring/my_io_uring.cpp ../../../reactor/uring/my_io_uring.h
        )

add_executable(ppclient  ${SOURCE_FILES}  ppclient.cpp)
target_link_libraries(ppclient ${OpenCV_LIBS})
