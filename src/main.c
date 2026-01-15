#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main(int argc, char *argv[]){

    if(argc != 2){

        printf("Please write: server (port)");
        return 1; 

    }

    int port = atoi(argv[1]);

    Server server =  server_constructor(port);

    printf("Server started successfully on port %d\n", port);

    return 0; 

}