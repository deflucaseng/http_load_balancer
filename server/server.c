#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define QUEUE_SIZE 10

// HTTP response template
const char* response_template = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

void handle_client(int client_socket, const char* server_id) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE);

    // Create response message
    char message[256];
    snprintf(message, sizeof(message), "Hello from server instance %s!\n", server_id);
    
    // Format full HTTP response
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), response_template, 
             strlen(message), message);

    // Send response
    write(client_socket, response, strlen(response));
    close(client_socket);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <server_id>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char* server_id = argv[2];

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }

    // Configure server address
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen for connections
    if (listen(server_socket, QUEUE_SIZE) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server %s listening on port %d...\n", server_id, port);

    while (1) {
        struct sockaddr_in client_addr = {0};
        socklen_t client_addr_len = sizeof(client_addr);

        // Accept connection
        int client_socket = accept(server_socket, 
                                 (struct sockaddr *)&client_addr, 
                                 &client_addr_len);
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        handle_client(client_socket, server_id);
    }

    close(server_socket);
    return 0;
}