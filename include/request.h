#ifndef REQUEST_H
#define REQUEST_H

#define BUFFER_SIZE 4096

typedef struct {
    char method[16];
    char path[256];
    char raw[BUFFER_SIZE];
} Request;

int parse_request(int client_socket, Request *request);

#endif