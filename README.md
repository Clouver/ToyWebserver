# ToyWebserver
A toy webserver for c++ practicing.

2022/08/14 开始

# Day1
  1、珠玉在前，先找资料学习 Reactor 模式，间中观摩一下 muduo 代码，互相参照。<br>
  2、新建文件夹 :)<br>
  
# Day2
  1、熟悉C++11的多线程开发，比c posix好用一百倍。
  2、尝试写 EventLoop Channel Poller，比想象中困难很多。理论上能理解 Reactor 模型，但是实际开发起来各个部分之间的关系很难理清。例如 Channel 和 EventLoop、Poller 都有关系，在哪里创建、怎么去销毁等等，并不是单独对着Channel就能写的。最后结果就是每个类设计实现了一部分，都不完整。目前思路是尝试不实现完整的reactor，复杂的地方先简化，以能跑起来为首要目的。
