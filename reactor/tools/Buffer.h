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
    char *buf_;
    size_t sz_;
    size_t capa_;
public:
    Buffer():buf_(new char[4096]), sz_(0), capa_(4096){
    }
    ~Buffer(){
        delete [] buf_;
    }

    size_t capacity()const{
        return capa_;
    }

    size_t size() const{
        return sz_;
    }

    void clear(){
        sz_ = 0;
    }

    int reserve(size_t sz){
        if(sz <= sz_)
            return -1;
        char * tmp = new char[sz];
        std::copy(buf_, buf_+sz_, tmp);
        delete [] buf_;
        std::swap(buf_, tmp);
        capa_ = sz;
        return 0;
    }

    void append(const char *tmp, size_t len){
        while(sz_ + len > capa_)
            reserve(capa_*2);
        std::copy(tmp, tmp+len, buf_+sz_);
        sz_ += len;
    }

    char operator[](size_t i) const{
        return buf_[i];
    }

    char* buf(){
        return buf_;
    }

    std::string toString(int idx=0, size_t len=0) const{
        if(len == 0)
            len = sz_ - idx;
        std::string ret;
        for(size_t i=idx; i<idx+len; i++)
            ret += buf_[i];
        return ret;
    }

};


#endif //TOYWEBSERVER_BUFFER_H
