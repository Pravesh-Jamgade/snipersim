#ifndef CQ_H
#define CQ_H
#include "fixed_types.h"
#include <bits/stdc++.h>

// #define MAX_QUEUE_SIZE 30
// class CQ
// {
// private:
//     IntPtr queue[MAX_QUEUE_SIZE];
//     int front;
//     int rear;
    
    
// public:
//     int ptr;
//     int count;

//     CQ()
//     {
//         front = rear = 0;
//         count = 0;
//         ptr = -1;
//     }

//     bool isEmpty()
//     {
//         return count == 0;
//     }

//     bool isFull()
//     {
//         return count == MAX_QUEUE_SIZE;
//     }

//     void enqueue(IntPtr va_address)
//     {
//         if (isFull())
//             dequeue();

//         queue[rear] = va_address;
//         rear = (rear + 1) % MAX_QUEUE_SIZE;
//         count++;
//     }

//     void dequeue()
//     {
//         if (isEmpty())
//             return;

//         front = (front + 1) % MAX_QUEUE_SIZE;
//         count--;
//     }

//     int get(int index)
//     {
//         if (index < 0 || index >= count)
//             return -1;

//         return queue[(front + index) % MAX_QUEUE_SIZE];
//     }
// };

#endif