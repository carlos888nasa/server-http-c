#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

#define NUM_COLA 1024

Server server_constructor(int port){

    Server server;
    server.port = port; 

    server.socket = socket(AF_INET, SOCK_STREAM, 0);

    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(port); 

    bind(server.socket, (struct sockaddr*)&server.address, sizeof(server.address));

    listen(server.socket, NUM_COLA);

    return server; 

}