#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../include/server.h"
#include "../include/request.h"
#include "../include/router.h"
#include "../include/response.h"

int global_server_socket = -1; 

// Graceful shutdown handler
void handle_sigint(int sig) {
    const char *msg = "\n\n Caught signal %d (Ctrl+C). Shutting down gracefully...\n";
    write(STDOUT_FILENO, msg, snprintf(NULL, 0, msg, sig)); // Async-safe logging
    
    if (global_server_socket != -1) {
        close(global_server_socket);
        const char *msg2 = " Server socket closed successfully. Port is now free.\n";
        write(STDOUT_FILENO, msg2, strlen(msg2)); // Async-safe logging
    }
    
     const char *msg3 = "Goodbye!\n";
    write(STDOUT_FILENO, msg3, strlen(msg3)); // Async-safe logging
    (void)sig; // Silence unused parameter warning
    exit(0); // Terminate the program safely
}

// Main server loop: Keeps the server running forever
void server_start(Server *server){

    global_server_socket = server->socket;
    signal(SIGINT, handle_sigint);

    printf("Server is running and waiting for connections...\n");
    printf("Press Ctrl+C to stop the server safely.\n\n");

    while(1) {
        
        // 1. Accept incoming client connection
        int client_socket = accept(server->socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // 2. Parse the incoming request using our Request module
        Request req = {0};
        if (parse_request(client_socket, &req) == 0) {
            
            // 3. Route the request to a physical file path using our Router module
            char real_file_path[512] = {0};
            route(&req, real_file_path, sizeof(real_file_path));

            // 4. Send the correct response back to the client using our Response module
            send_response(client_socket, real_file_path);
        }

        // 5. Close connection
        close(client_socket);
    }
}

// Server initialization and socket binding
Server server_constructor(int port){

    Server server;
    server.port = port; 

    // Create TCP socket
    server.socket = socket(AF_INET, SOCK_STREAM, 0);

    // SO_REUSEADDR prevents "Address already in use" errors after restart
    int opt = 1;
    setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Define address structure
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(port); 

    // Always check if bind() succeeds
    if (bind(server.socket, (struct sockaddr*)&server.address, sizeof(server.address)) < 0) {
        perror("Bind failed (Is the port already in use?)");
        exit(EXIT_FAILURE);
    }

    // Always check listen() and use SOMAXCONN for dynamic OS scaling
    if (listen(server.socket, SOMAXCONN) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    return server; 
}