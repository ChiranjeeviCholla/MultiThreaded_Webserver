#include "ThreadPool.h"
#include <unistd.h>   // close()
#include <sys/socket.h>
#include <cstring>

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

void ThreadPool::enqueue(int clientSocket) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(clientSocket);
    }
    condition.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        int clientSocket;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] {
                return stop || !tasks.empty();
            });

            if (stop && tasks.empty())
                return;

            clientSocket = tasks.front();
            tasks.pop();
        }

        // ---- Handle client ----
        char buffer[1024] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);

        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";

        send(clientSocket, response, strlen(response), 0);
        close(clientSocket);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }

    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}
