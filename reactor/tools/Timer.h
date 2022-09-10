//
// Created by Clouver on 2022/9/9.
//

#ifndef TOYWEBSERVER_TIMER_H
#define TOYWEBSERVER_TIMER_H

#include <chrono>
#include <string>
#include <iostream>
//#include <utility>
#include <functional>
#include <csignal>
#include <syscall.h>

using namespace std::chrono;
using namespace std;

class Timer {
    string name_;
    steady_clock::time_point timer;
    double costSum;
    size_t cnt;

public:
    Timer():Timer(""){
        timer = steady_clock::now();
    }
    explicit Timer(string name):name_(std::move(name)), timer(steady_clock::now()), costSum(0.0), cnt(0){
        timer = steady_clock::now();
    }

    void runInTiming(const function<void()>& f){
        tick();
        f();
        tock();
    }

    void tick(){
        reset();
    }
    double tock(){
        double t = duration_cast<duration<double>>(steady_clock::now() - timer).count();
        costSum += t;
        cnt++;
        if(cnt != 0 && (cnt % 10000) == 0 && name_!=""){
//            cout<<::syscall(SYS_gettid)<<endl;
//            cout << "Timer "<< name_ <<" average time cost: " << costSum/static_cast<double>(cnt) << endl;
        }
        return t;
    }

    void reset(){
        timer = steady_clock::now();
    }

    ~Timer(){
        if(name_!="")
            cout << name_ <<" average time cost: " << costSum/static_cast<double>(cnt) << endl;
    }
};


#endif //TOYWEBSERVER_TIMER_H
