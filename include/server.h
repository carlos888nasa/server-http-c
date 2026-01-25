#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <netinet/ip.h> 

typedef struct{

    int socket;
    int port;
    struct sockaddr_in address ;
    
}Server;

void server_start(Server *server);

Server server_constructor(int port);

const char* get_content_type(const char *path);

#endif 