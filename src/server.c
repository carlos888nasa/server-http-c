#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"

#define NUM_COLA 1024
#define BUFFER_SIZE 4096

// Main server loop: Keeps the server running forever
void server_start(Server *server){

    while(1) {

        // 1. Accept incoming client connection
        int client_socket = accept(server->socket, NULL, NULL);

        // Buffers for HTTP parsing (initialized to 0 for safety)
        char buffer[BUFFER_SIZE] = {0};
        char method[16];
        char path[256];
        char file_path[512];
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // 2. Read the HTTP request from the browser
        recv(client_socket, buffer, sizeof(buffer) - 1, 0);  
        printf("Received request:\n%s\n", buffer);

        // 3. Extract HTTP Method (GET) and URL Path (/ or /api)
        sscanf(buffer, "%s %s", method, path);

        // 4. Default routing: redirect root to index.html
        if(!strcmp(path, "/")){

            strcpy(path, "/index.html");

        }

        // 5. Build the physical file path (e.g., "./www/index.html")
        sprintf(file_path, "./www%s", path);

        // 6. Try to open the requested file
        FILE *file = fopen(file_path, "r");

        if(file != NULL){ 

            printf("✅ Serving file...\n");

            char file_content[BUFFER_SIZE] = {0};
            char http_header[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            char response[BUFFER_SIZE * 2] = {0};

            fread(file_content, 1, BUFFER_SIZE, file);

            strcat(response, http_header);
            strcat(response, file_content);

            send(client_socket, response, strlen(response), 0);

            fclose(file);

        }else {

            printf("❌ 404 Not Found\n");
            FILE *not_found_file = fopen("www/error_404.html", "r");

            if(not_found_file != NULL){

                char file_content[BUFFER_SIZE] = {0};
                char http_header[BUFFER_SIZE] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
                char response[BUFFER_SIZE * 2] = {0};

                fread(file_content, 1, BUFFER_SIZE, not_found_file);

                strcat(response, http_header);
                strcat(response, file_content);

                send(client_socket, response, strlen(response), 0);

                fclose(not_found_file);
            }
        }

        // 7. Close connection
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