#include "Server.h"
#include "Threadpool.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#define PORT 8080

void startServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    ThreadPool pool(4);  // 👈 FIXED number of threads

    std::cout << "Server running on port 8080\n";

    while (true) {
        int clientSocket = accept(server_fd, nullptr, nullptr);
        pool.enqueue(clientSocket);
    }

    close(server_fd);
}
