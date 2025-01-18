#pragma once
#include <string>
#include <utility>

template <typename T>
class Queue {
public:
    virtual ~Queue() {}
   
    virtual bool isFull() const =0;
    virtual bool isEmpty() const = 0;
    virtual void enqueue(const T& item) = 0;
    virtual T dequeue() =0 ;
};