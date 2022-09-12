# ToyWebserver
A toy webserver for c++ practicing.</br>

### start
2022/08/14 开始<br>
2022/08/18 基础完工</br>
服务器演示已经部署。</br>
暂时告一段落复习基础知识准备面试。</br>

### I/O bench
2022/08/24 PingPong </br>
增加点工作量，开始做 PingPong I/O bench</br>
2022/08/25 完成 <br>
但是跑满只有 20MB/s ，相比编译运行 muduo 单连接就有50+，很不合理。初步推断是读写缓存问题。笔试面试有点多，项目暂缓。</br>

2022/09/10 **读写效率排查**
（面试得到反馈，原来webserver竟是如此烂大街之物，好像是培训机构会教这个？但是有比没有好，还是继续做吧。）<br>
写了个耗时分析的小工具，排查发现确实是读写效率问题，虽然不是想象中的那种。</br>
2022/09/11 **修复IO效率问题**</br>
吞吐量还有提升空间但是不大，暂缓。</br>

### 内存泄漏
2022/09/11 ** Valgrind 检查内存泄漏**<br>
检查出 subreactor 线程的退出问题。其实当初就加了todo tag - -</br>
2022/09/11 **修复内存泄漏**</br>
除了 opencv 的库调用实在是会留一些 **reachable at exit** 之外，我自写的没有内存问题。<br>

### 压力测试
2022/09/12 **未知问题**<br>
windows 用 jmeter 测 wsl2 上跑的服务器，直接寄。此后任何连接都被拒绝，重启server+换端口无用。<br>
当然推测是防火墙问题，或许压测被当成了某种攻击。<br>
`sudo iptables -P INPUT ACCEPT` 放开防火墙无用。windows防火墙显然无用。<br>
没找到资料。先就用bash跑，把log文件复制回windows的gui看结果。<br>

2022/09/12 **fd数量限制问题**<br>
进程存在打开文件数量限制。首先显然调整这个限制。<br>
应当给文件做**内存中的缓存**，不真正打开文件。待完善<br>
其次，**socket 会占用fd**，如果使用正常关闭`shutdown( fd, SHUT_WR )`，会保留连接一定时间。但也没太大必要优化这部分。<br>
管理员权限下可修改 hard limit(4096) 和 soft limit(1024)。`setrlimit64(RLIMIT_NOFILE, &lm)`<br>

### 线程安全
2022/09/12 **`std::map` 线程安全性**
众所周知 11 标准的 `map` 具有线程安全性，包括多线程读写不同element。<br>
然而，`operator[]` 和 `erase` 都会使得iterator失效，导致多线程操作时的undefined behavior<br>
实现无锁hashmap连接池，解决创建、销毁连接时的线程安全问题。<br>

### 静态资源Buffering
2022/09/12 **TPS太低** <br>
压测发现静态资源访问 TPS 只有1000。网上查到 E5‑2699 v3 跑静态资源 SSL TPS 可以到 10000。<br>
第一想法是，每次请求读写磁盘文件耗时太多。分析了一下耗时，印证确实如此。<br>
实现了非常非常简陋的静态文件buffer，重测得到3000TPS。<br>
查看 cpu 占用，发现 jmeter 跑到 600%，server只有不到 70%。todo <br>





