#include "Threadpool.h"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

/* ---------- MIME TYPE HELPERS (C++17 SAFE) ---------- */

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::string getMimeType(const std::string& path) {
    if (endsWith(path, ".html")) return "text/html";
    if (endsWith(path, ".css"))  return "text/css";
    if (endsWith(path, ".js"))   return "application/javascript";
    if (endsWith(path, ".png"))  return "image/png";
    if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
    if (endsWith(path, ".gif"))  return "image/gif";
    return "application/octet-stream";
}

/* ---------- THREAD POOL IMPLEMENTATION ---------- */

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

        /* ---------- READ HTTP REQUEST ---------- */

        char buffer[4096] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            close(clientSocket);
            continue;
        }

        std::string request(buffer);
        size_t pos = request.find("\r\n");
        if (pos == std::string::npos) {
            close(clientSocket);
            continue;
        }

        std::string requestLine = request.substr(0, pos);
        std::istringstream iss(requestLine);

        std::string method, path, version;
        iss >> method >> path >> version;

        /* ---------- METHOD CHECK ---------- */

        if (method != "GET") {
            const char* response =
                "HTTP/1.1 405 Method Not Allowed\r\n"
                "Content-Length: 0\r\n\r\n";
            send(clientSocket, response, strlen(response), 0);
            close(clientSocket);
            continue;
        }

        if (path == "/") {
            path = "/index.html";
        }

        /* ---------- FILE MAPPING ---------- */

        std::string filePath = "./www" + path;
        std::ifstream file(filePath, std::ios::binary);

        if (!file) {
            const char* response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "\r\n"
                "404 Not Found";
            send(clientSocket, response, strlen(response), 0);
            close(clientSocket);
            continue;
        }

        /* ---------- READ FILE ---------- */

        std::string body(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        std::string mimeType = getMimeType(filePath);

        /* ---------- SEND RESPONSE ---------- */

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: " + mimeType + "\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "\r\n";

        send(clientSocket, response.c_str(), response.size(), 0);
        send(clientSocket, body.c_str(), body.size(), 0);

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
