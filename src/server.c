#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../include/server.h"
#include "../include/request.h"
#include "../include/router.h"
#include "../include/response.h"
#include "../include/threapool.h"

#define DEFAULT_QUEUE_SIZE 1024

int global_server_socket = -1; 
ThreadPoolManager *pool = NULL;

// Graceful shutdown handler
void handle_sigint(int sig) {
    const char *msg1 = "\n\n Caught signal (Ctrl+C). Shutting down gracefully...\n";
    const char *msg2 = " Server socket closed successfully. Port is now free.\n";
    const char *msg3 = "Goodbye!\n";

    write(STDOUT_FILENO, msg1, strlen(msg1)); // Async-safe logging

    if(pool != NULL) {
        threadpool_destroy(pool);
        write(STDOUT_FILENO, " Thread pool destroyed successfully.\n", 36); // Async-safe logging
    }
    
    if (global_server_socket != -1) {
        close(global_server_socket);
        write(STDOUT_FILENO, msg2, strlen(msg2)); // Async-safe logging
    }
    
    write(STDOUT_FILENO, msg3, strlen(msg3)); // Async-safe logging
    (void)sig; // Silence unused parameter warning
    _exit(0); // Terminate the program safely
}

void handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg); // Free the dynamically allocated socket descriptor

    // Parse the incoming request using our Request module
    Request req = {0};
    if (parse_request(client_socket, &req) == 0) {
        
        // Route the request to a physical file path using our Router module
        char real_file_path[512] = {0};
        route(&req, real_file_path, sizeof(real_file_path));

        // Send the correct response back to the client using our Response module
        send_response(client_socket, real_file_path);
    }

    // Close connection
    close(client_socket);
}

// Main server loop: Keeps the server running forever
void server_start(Server *server){

    global_server_socket = server->socket;
    signal(SIGINT, handle_sigint);

    long thread_count = sysconf(_SC_NPROCESSORS_ONLN); // Optimal thread count based on CPU cores
    printf("Detected %ld CPU cores. Starting thread pool with %ld threads.\n", thread_count, thread_count);
    pool = threadpool_create(thread_count, DEFAULT_QUEUE_SIZE);

    if(pool == NULL) {
        fprintf(stderr, "[FATAL ERROR] Failed to create thread pool with %ld threads: %s\n", thread_count, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections...\n");
    printf("Press Ctrl+C to stop the server safely.\n\n");

    while(1) {
        
        // 1. Accept incoming client connection
        int client_socket = accept(server->socket, NULL, NULL);
        if (client_socket < 0) {
            printf("[ERROR] Failed to accept connection: %s\n", strerror(errno));
            continue;
        }

        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL) {
            printf("[ERROR] Failed to allocate memory for client socket: %s\n", strerror(errno));
            close(client_socket);
            continue;
        } 

        *client_socket_ptr = client_socket;

        if (threadpool_add(pool, handle_client, client_socket_ptr) != 0) {
            const char *busy_msg = "HTTP/1.1 503 Service Unavailable\r\n\r\nServer is currently full.";
            write(client_socket, busy_msg, strlen(busy_msg));
            close(client_socket);
            free(client_socket_ptr);
        }
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