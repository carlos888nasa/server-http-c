#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <netinet/ip.h> 

typedef struct{

    int socket;
    int port;
    struct sockaddr_in address ;
    
}Server;

Server server_constructor(int port);

#endif 