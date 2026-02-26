#include <cstring>
#include <iostream>
#include <string>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    hostname[255] = '\0';

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "ERROR: Failed to create socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to port 8080 on all interfaces
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "ERROR: Failed to bind to port 8080\n";
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "ERROR: Failed to listen on port 8080\n";
        return 1;
    }

    std::cout << "Server listening on port 8080 (hostname: "
              << hostname << ")\n";

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        // Prepare response body
        std::string body = "Served by backend: " + std::string(hostname) + "\n";

        // Prepare full HTTP response
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(body.length()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;

        // Ensure full send
        size_t total_sent = 0;
        while (total_sent < response.size()) {
            ssize_t sent = send(client_fd,
                                response.c_str() + total_sent,
                                response.size() - total_sent,
                                0);
            if (sent <= 0) break;
            total_sent += sent;
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
