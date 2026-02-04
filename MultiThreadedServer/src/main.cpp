#include <iostream>
#include <cstring>
#include <unistd.h>         // close()
#include <arpa/inet.h>      // sockaddr_in
#include <sys/socket.h>     // socket(), bind(), listen(), accept()

int main() {
    // 1️⃣ Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    // 2️⃣ Bind socket to IP + Port
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   // 0.0.0.0
    address.sin_port = htons(8080);         // Host → Network byte order

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    // 3️⃣ Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    // 4️⃣ Accept ONE client (blocking)
    int client_fd = accept(server_fd, nullptr, nullptr);

    if (client_fd < 0) {
        perror("accept failed");
        close(server_fd);
        return 1;
    }

    // 5️⃣ Send response
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World";

    send(client_fd, response, strlen(response), 0);

    // 6️⃣ Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}

