//
// Created by Clouver on 2022/8/14.
//

#include <condition_variable>
#include <mutex>
#include <bits/stdc++.h>

#include <thread>
#include <functional>

using namespace std;

queue<int>q;
mutex m;
condition_variable cond;

void func1(void){
    int i=0;
    while(i<30)
    {
        unique_lock<mutex> lock(m);
        while(q.empty())
            cond.wait(lock);
        cout<<q.front()<<endl;
        q.pop();
        i++;
    }
}
void func2(int x){
    int i=0;
    while(i<20)
    {
        unique_lock<mutex> lock(m);
        q.push(x);
        lock.unlock();
        cond.notify_one();
        i++;
        this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main(void){
    thread t1(bind(func2, 1 )), t2(bind(func2, 2 )), consumer(func1);
    t1.join();
    t2.join();
    consumer.join();
}