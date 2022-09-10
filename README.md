# ToyWebserver
A toy webserver for c++ practicing.

2022/08/14 开始<br>

2022/08/18 服务器演示已经部署。暂时告一段落复习基础知识准备面试。<br>


// todo 项目简介
// todo bench


poolAndHandle   0.0002

poll            0.0001
loopForHandling 0.0001

io per msg      0.000002

---------------------下面是混乱的notes---
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
  2、成功跑起来 echo 服务。 为啥感觉ping起来延迟有点高，能感觉到约0.1s。可能是 WSL 或者 powershell 屏幕打印延迟，也可能是服务器出大问题。下一步先测一下延迟再搞其他。<br>
  3、测试完延迟之后，最紧要的应该是loop的进一步完善。现在实现的loop是简单的自旋式，必然要做个信号触发机制，没有新epoll事件|新注册channel的时候阻塞，有新epoll事件处理epoll事件，有新channel时唤醒处理channel。
  
# Day4
  1、延迟没有问题。<br>
  2、计划有变，tcp连接的关闭的部分太碍眼，优先处理。 实现方式是单独设置一个关闭tcpconnection的thread，server启动时启动，关闭的fd放入队列作为临界区供subreactor安全访问。<br>
  3、去看了muduo源码，这部分是利用 runinloop 做到线程安全，一行就搞定了。而且loop的阻塞与触发也可以这样一并解决！ 之前看见里面functor queue相关的代码，以为是为了作为库的通用性存在的，我实现webserver是特化的可能用不上，现在看来是想岔了。（有种偷看答案的感觉。）<br>
  4、shared_ptr 与 bind。 bind 的本质是一个functor！ 如果传入 shared_ptr，会导致引用计数+1，bind(func,A) 设置为 B 的回调函数会间接循环引用。

# Day5
  1、算是基本完成，开始改进缺陷和添加http相关内容。如今还是echo服务器。<br>
  2、关于连接关闭的部分，muduo是把事件机制应用到底，eventloop那里存了channel，那就给eventloop添加任务让它自己去关闭。确实很简洁。 我目前的方式是单开一个关闭连接线程，看似不会影响主reactor的效率，但实际上这个关闭连接线程要等待 eventloop 的资源，假设一直阻塞，就会影响其他多个连接的关闭。然而eventloop的临界区涉及的操作都非常简单，往 map 里insert 和 从 map 里erase，不包含其它可能的阻塞或者大量计算任务，似乎没必要考虑那种极端情况。<br>
  3、编译安装opencv，给picTochar功能做准备。<br>
  4、实现 multipart 功能。<br>
  5、实现picTochar接口。<br>
  6、麻了，前端真的难搞。<br>
 
# Day6
  1、基本完工，下一步整理一下代码，还有部署，还有项目介绍。<br>
  2、已部署，辣鸡VPS一个pictochar请求要1秒钟。<br>
  3、准备简历 准备背书 准备面试。<br>
  4、访问部署的服务器发现非常慢，相比210ms的延迟来说有点过分，本地调试一下，发现连localhost会有 约300ms 的延迟，而且是一个页面多个请求，一个300ms一个5ms。<br>
  反复刷新，每次都是一个100~300ms，另一个3-5ms，文件名并不影响。本地怎么会有300ms？而且时有时不有。<br>
  调了一晚上，最后把地址从localhost改成127.0.0.1就没延迟了。<br>
  细看Chrome 连接延迟的细分子类，发现是 initial connection部分很慢。<br>
  乖乖，访问地球另一边的网站这个子类都是0，localhost专属延迟啊？<br>
  真的麻完了给我。<br>
