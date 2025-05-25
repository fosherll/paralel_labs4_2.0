#ifndef LAB_4_2_0_THREADPOOL_H
#define LAB_4_2_0_THREADPOOL_H

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <iostream>
#include "Task.h"
#include <winsock2.h>

class ThreadPool {
private:
    std::vector<std::thread> massiveThread;
    std::queue<Task> queueTask;
    std::mutex printConsoleMutex;
    std::mutex taskQueueMutex;
    std::condition_variable startTask;
    bool stop=false;

    void TakeTask() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(taskQueueMutex);
                startTask.wait(lock, [this] { return stop || !queueTask.empty(); });

                if (stop && queueTask.empty())
                    return;

                task = std::move(queueTask.front());
                queueTask.pop();
            }

            if (task.validation()) {
                MakeTask(task);
            }
        }
    }

    void MakeTask(const Task& task) {
        printConsoleMutex.lock();
            std::cout << "Task " << task.id << " started" << std::endl;
        printConsoleMutex.unlock();

            task.task();
        printConsoleMutex.lock();
            std::cout << "Task " << task.id << " is ready" << std::endl;
        printConsoleMutex.unlock();
    }

public:
    ThreadPool(int sizeThread) : stop(false) {
        for (int i = 0; i < sizeThread; ++i) {
        massiveThread.emplace_back(&ThreadPool::TakeTask, this);
    }
}

    ~ThreadPool() {
        stop = true;
        startTask.notify_all();

        for (auto& t : massiveThread) {
            if (t.joinable())
                t.join();
        }
    }

    void addTask(Task newTask) {
            taskQueueMutex.lock();
            queueTask.emplace(std::move(newTask));
            taskQueueMutex.unlock();
        startTask.notify_one();
    }
};

#endif //LAB_4_2_0_THREADPOOL_H
