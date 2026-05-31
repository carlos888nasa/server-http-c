#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/request.h"

int parse_request(int client_socket, Request *request) {
    char buffer[BUFFER_SIZE] = {0};

    ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        return -1; // Error or connection closed
    }
    request->raw[received] = '\0'; // Null-terminate the received data

    printf("Received request:\n%s\n", request->raw);
    sscanf(request->raw, "%15s %255s", request->method, request->path);

    char *q = strchr(request->path, '?');  if (q) *q = '\0';
    char *cr = strchr(request->path, '\r'); if (cr) *cr = '\0';
    char *nl = strchr(request->path, '\n'); if (nl) *nl = '\0';

    if (strstr(request->path, "..")) {
        const char *bad_req = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request: Nice try, hacker!</h1>";
        send(client_socket, bad_req, strlen(bad_req), 0);
        return -1; // Abort connection
    }

    return 0; // Success
}
