#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void enqueue(int clientSocket);

private:
    void workerLoop();

    std::vector<std::thread> workers;
    std::queue<int> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

#endif
