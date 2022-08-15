# ToyWebserver
A toy webserver for c++ practicing.

2022/08/14 开始

# Day1
  1、珠玉在前，先找资料学习 Reactor 模式，间中观摩一下 muduo 代码，互相参照。<br>
  2、新建文件夹 :)<br>
  
# Day2
  1、熟悉C++11的多线程开发，比c posix好用一百倍。<br>
  2、尝试写 EventLoop Channel Poller，比想象中困难很多。理论上能理解 Reactor 模型，但是实际开发起来各个部分之间的关系很难理清。<br>
  例如 Channel 和 EventLoop、Poller 都有关系，在哪里创建、怎么去销毁等等，并不是单独对着Channel就能写的。最后结果就是每个类设计实现了一部分，都不完整。<br>
  目前思路是尝试不实现完整的reactor，复杂的地方先简化，以能跑起来为首要目的。

# Day3
  1、调网络编程的bug有点折磨人。太多细节要注意。<br>
  主要遇到的两个坑：<br>正确理解EPOLLOUT，不要在LT设置EPOLLOUT，缓冲区一直空那就一直有EPOLLOUT信号。<br>
  read write默认是阻塞的，要把fd设置为非阻塞。居然对这个没印象了。<br>
  2、成功跑起来 echo 服务。 为啥感觉ping起来延迟有点高，能感觉到约0.1s。可能是 WSL 或者 powershell 屏幕打印延迟，也可能是服务器出大问题。下一步先测一下延迟再搞其他。
  3、测试完延迟之后，最紧要的应该是loop的进一步完善。现在实现的loop是简单的自旋式，必然要做个信号触发机制，没有新epoll事件|新注册channel的时候阻塞，有新epoll事件处理epoll事件，有新channel时唤醒处理channel。
