#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"

#define NUM_COLA 1024
#define BUFFER_SIZE 4096

void server_start(Server *server){

    while(1) {

        int client_socket = accept(server->socket, NULL, NULL);
        char buffer[BUFFER_SIZE] = {0};
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        recv(client_socket, buffer, sizeof(buffer) - 1, 0);  
        printf("Received request:\n%s\n", buffer);

        char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello World!";

        send(client_socket, response, strlen(response), 0);

        close(client_socket);
    }
}

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