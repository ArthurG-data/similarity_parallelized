#pragma once

#include <assert.h>
#include <windows.h>
#include "Queue.h"
#include <vector>


template <typename T>
class ThreadSafeQueue : public Queue<T> {
private:
    bool shutdown;
    std::vector<T> data;
    int maxSize;
    int  front;
    CONDITION_VARIABLE notFull, notEmpty;
    CRITICAL_SECTION lock;

public:
    ThreadSafeQueue(int maxSize) : maxSize(maxSize), shutdown(false)
    {
        
        data.reserve(maxSize);
        front =  -1;
        InitializeCriticalSection(&lock);
        InitializeConditionVariable(&notFull);
        InitializeConditionVariable(&notEmpty);

    }
    ~ThreadSafeQueue() 
    {
        DeleteCriticalSection(&lock);
    }

    bool isFull() const override 
    {
        return front >= maxSize;
    }

    bool isEmpty() const override 
    {
        return front == -1;
    }
    void enqueue(const T& item) override 
    {
  
        EnterCriticalSection(&lock);
   
        while (!shutdown && isFull()) 
        {
         
            SleepConditionVariableCS(&notFull, &lock, INFINITE);
        }
        if (shutdown) {
            LeaveCriticalSection(&lock);
            return;
        }
        assert(!isFull());

        if (front == -1)//special case, init
            front = 0;
        
        data.push_back(item); // Fill existing space
        front =  front +1 ;
        

        LeaveCriticalSection(&lock);
        WakeConditionVariable(&notEmpty);


    }
    T dequeue() override 

    {
        EnterCriticalSection(&lock);

        while (isEmpty() && !shutdown)
            SleepConditionVariableCS(&notEmpty, &lock, INFINITE); // Wait for an item to be available

        if (shutdown && isEmpty()) {
            LeaveCriticalSection(&lock);
            return T(); // Return a default-constructed T
        }
        assert(!isEmpty());

        T item = data.back();
        data.pop_back();
        front--;
        if (front == 0)//last item
            front = -1;//queue is empty


        LeaveCriticalSection(&lock);
        WakeConditionVariable(&notFull);

        return item;
    }
        void setShutDown(bool value) 
        {
            shutdown = value;
        }
        bool getShutDown() const
        {
            return shutdown;
        }
        
};


