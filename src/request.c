#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "../include/request.h"

int parse_request(int client_socket, Request *request) {

    struct timeval timeout = {5, 0}; // 5 seconds timeout
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
   
    ssize_t received = recv(client_socket, request->raw, sizeof(request->raw) - 1, 0);
    if (received <= 0) {
        return -1; // Error or connection closed
    }
    request->raw[received] = '\0'; // Null-terminate the received data

    printf("Received request:\n%.80s\n", request->raw);
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
