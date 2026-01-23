#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main(int argc, char *argv[]){

    if(argc != 2){

        printf("Please write: server (port)\n");
        return 1; 

    } 

    int port = atoi(argv[1]);

    Server server =  server_constructor(port);

    server_start(&server);

    return 0; 

}