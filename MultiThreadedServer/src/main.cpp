#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void handle_client(int client_fd) {
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World";

    send(client_fd, response, strlen(response), 0);
    close(client_fd);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Multithreaded server listening on port 8080...\n";

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        std::thread t(handle_client, client_fd);
        t.detach();
    }

    close(server_fd);
    return 0;
}
