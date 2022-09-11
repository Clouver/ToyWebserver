//
// Created by Clouver on 2022/9/10.
//

#ifndef TOYWEBSERVER_BUFFER_H
#define TOYWEBSERVER_BUFFER_H


#include <cstddef>
#include <vector>
#include <string>

using std::vector;

class Buffer {
private:
    vector<char>buf_;
    size_t sz_;

public:
    Buffer():buf_(4096), sz_(0){
    }

    void clear(){
        sz_ = 0;
        buf_.clear();
    }

    void append(const char *tmp, size_t len){
        copy(tmp, tmp+len, std::back_inserter(buf_));
//        while(sz_ + len > buf_.capacity())
//            buf_.reserve(buf_.capacity() * 2);
//        for(size_t i=0; i<len; i++)
//            buf_[i+sz_] = tmp[i];
//        std::copy(tmp, tmp+len, &*buf_.begin() + sz_);
        sz_ += len;
    }

    size_t size() const{
        return sz_;
    }

    char operator[](size_t i) const{
        return buf_[i];
    }

    char* buf(){
        return &*buf_.begin();
    }

    std::string toString(int idx=0, size_t len=0) const{
        if(len == 0)
            len = sz_ - idx;
        std::string ret;
        for(int i=idx; i<idx+len; i++)
            ret += buf_[i];
        return std::move(ret);
    }

};


#endif //TOYWEBSERVER_BUFFER_H
