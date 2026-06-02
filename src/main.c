#include <stdio.h>
#include <stdlib.h>

#include "../include/server.h"

int main(int argc, char *argv[]){

    if(argc != 2){

        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);

    } 

    int port = atoi(argv[1]);
    Server server =  server_constructor(port);
    server_start(&server);

    return 0; 

}