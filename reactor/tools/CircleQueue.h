//
// Created by Clouver on 2022/9/10.
//

#ifndef TOYWEBSERVER_CIRCLEQUEUE_H
#define TOYWEBSERVER_CIRCLEQUEUE_H

#include <cstdlib>
#include <array>
#include <exception>
#include <vector>

template<class T>
class CircleQueue{
private:
    size_t rear_;
    size_t front_;
    size_t size_;
//    std::vector<T>q;
    T *q;

public:
    CircleQueue() = delete;
    explicit CircleQueue(size_t queueSize);

    int push(const T& t);

    int pop();

    T front();

    bool empty();

    ~CircleQueue();
};


template<class T> CircleQueue<T>::CircleQueue(size_t queueSize):rear_(0), front_(0),
                                                                size_(queueSize){
                                                                    q = new T[queueSize];
}

template<class T>
int CircleQueue<T>::push(const T& t){
    size_t nxt = (rear_ + 1) % size_;
    if (front_ == nxt)
        return -1;
    q[rear_] = t;
    rear_ = nxt;
    return 0;
}

template<class T>
int CircleQueue<T>::CircleQueue::pop(){
    size_t toPop = (front_ + 1) % size_;
    if(empty())
        return -1;
    q[front_] = nullptr;
    // ~T()
    front_ = toPop;
    return 0;
}

template<class T>
T CircleQueue<T>::CircleQueue::front(){
    if(empty())
        throw std::exception();
    return q[front_];
}

template<class T>
CircleQueue<T>::~CircleQueue(){
    delete [] q;
}

template<class T>
inline bool CircleQueue<T>::empty() {
    return rear_ == front_;
}


#endif //TOYWEBSERVER_CIRCLEQUEUE_H
