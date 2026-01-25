#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"

#define NUM_COLA 1024
#define BUFFER_SIZE 4096

const char* get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream"; // Default binary type

    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";

    return "application/octet-stream"; // Default binary type
}

// Main server loop: Keeps the server running forever
void server_start(Server *server){

    while(1) {

        // 1. Accept incoming client connection
        int client_socket = accept(server->socket, NULL, NULL);

        // Buffers for HTTP parsing (initialized to 0 for safety)
        char buffer[BUFFER_SIZE] = {0};
        char method[16];
        char path[256];
        char real_file_path[512] = {0};
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // 2. Read the HTTP request from the browser
        // ----- 2. Leer petición de forma SEGURA -----
        ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            close(client_socket);
            continue;
        }
        buffer[received] = '\0'; // Terminador nulo vital
        printf("Received request:\n%s\n", buffer);

        // ----- 3. Extraer method y path con límites de seguridad -----
        sscanf(buffer, "%15s %255s", method, path);

        // ----- 3.5. LIMPIEZA DE URL (Quitar query string y caracteres invisibles) -----
        char *q = strchr(path, '?');  if (q) *q = '\0';
        char *cr = strchr(path, '\r'); if (cr) *cr = '\0';
        char *nl = strchr(path, '\n'); if (nl) *nl = '\0';

        // ----- SEGURIDAD: Evitar Path Traversal (Hackers) -----
        if (strstr(path, "..")) {
            const char *bad_req = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request: Nice try, hacker!</h1>";
            send(client_socket, bad_req, strlen(bad_req), 0);
            close(client_socket);
            continue;
        }

        // ----- 4. AUTOCORRECCIÓN DE RUTAS (Si pones /index te lleva a /index.html) -----
        if (strcmp(path, "/") == 0) {
            strncpy(path, "/index.html", sizeof(path) - 1);
            path[sizeof(path)-1] = '\0';
        } else {
            char *last_slash = strrchr(path, '/');
            char *dot_after_slash = (last_slash) ? strchr(last_slash, '.') : NULL;
            // Si no hay un punto (.), asumimos que es un archivo HTML
            if (!dot_after_slash) {
                strncat(path, ".html", sizeof(path) - strlen(path) - 1);
            }
        }

        // ----- 5. CONSTRUCTOR DE RUTAS (El Enrutador Definitivo) -----
        if (!strncmp(path, "/api", 4)) {
            if (!strcmp(path, "/api") || !strcmp(path, "/api/")) {
                snprintf(real_file_path, sizeof(real_file_path), "./data/status.json");
            } else {
                snprintf(real_file_path, sizeof(real_file_path), "./data%s", path + 4);
            }
        } else {
            // Archivos estáticos
            snprintf(real_file_path, sizeof(real_file_path), "./www%s", path);
        }

        // DEBUG: Esto te salvará la vida para ver qué intenta abrir C
        printf("DEBUG -> Intentando abrir el archivo real: '%s'\n", real_file_path);

        // ==========================================
        // FASE 2: EL SERVIDOR (Una única vez)
        // ==========================================
        
        FILE *file = fopen(real_file_path, "r");
        if(file != NULL){

            printf("✅ Serving file: %s\n", real_file_path);

            char file_content[BUFFER_SIZE] = {0};
            char http_header[BUFFER_SIZE] = {0};

            // 1. Cabecera HTTP
            const char *content_type = get_content_type(real_file_path);
            sprintf(http_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
            send(client_socket, http_header, strlen(http_header), 0);

            // 2. BUCLE MAESTRO: Lee y envía en trozos de 4KB hasta el final del archivo
            size_t bytes_read;
            while ((bytes_read = fread(file_content, 1, BUFFER_SIZE, file)) > 0) {
                send(client_socket, file_content, bytes_read, 0);
            }

            fclose(file);

        } else {

            printf("❌ File not found: %s\n", real_file_path);

            FILE *file_404 = fopen("./www/error_404.html", "r");  

            if(file_404 != NULL){

                char file_content[BUFFER_SIZE] = {0};
                char http_header[BUFFER_SIZE] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";

                // 1. Cabecera HTTP
                send(client_socket, http_header, strlen(http_header), 0);

                // 2. BUCLE: Envío del archivo de error
                size_t bytes_read;
                while ((bytes_read = fread(file_content, 1, BUFFER_SIZE, file_404)) > 0) {
                    send(client_socket, file_content, bytes_read, 0);
                }

                fclose(file_404);
            } else {
                // Fallback de emergencia
                const char *not_found_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
                send(client_socket, not_found_response, strlen(not_found_response), 0);
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