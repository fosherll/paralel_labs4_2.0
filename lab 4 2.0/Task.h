//
// Created by foshe on 09.05.2025.
//

#ifndef LAB_4_2_0_TASK_H
#define LAB_4_2_0_TASK_H
#include <functional>
#include <winsock2.h>

class Task
{
public:
    int id;
    std::function<void()> task;
    Task()
    {
        this -> task = nullptr;
    }
    Task( int id, std::function<void()> task)
    {
        this -> id=id;
        this -> task=task;
    }
    bool validation() {
        if (this -> task != nullptr) {
            return true;
        }
         return false;
    }
};

#endif //LAB_4_2_0_TASK_H
