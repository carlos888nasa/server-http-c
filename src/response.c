#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "../include/response.h"
#include "../include/mime.h"
#include "../include/request.h"

void send_response(int socket, const char *real_file_path) {
    FILE *file = fopen(real_file_path, "rb"); // 'rb' for safe binary reading
    
    if (file != NULL) {
        printf("✅ Serving file: %s\n", real_file_path);

        fseek(file, 0, SEEK_END);      // Move the cursor to the end of the file
        long file_size = ftell(file);  // Get the current byte offset (file size)
        fseek(file, 0, SEEK_SET);      // Reset the cursor back to the beginning

        char http_header[BUFFER_SIZE] = {0};
        const char *content_type = get_content_type(real_file_path);
        
        // Create the dynamic header including the exact content length
        snprintf(http_header, sizeof(http_header), 
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %ld\r\n\r\n", 
                 content_type, file_size);
                 
        send(socket, http_header, strlen(http_header), 0);

        // File sending loop (Streaming chunks)
        char file_content[BUFFER_SIZE] = {0};
        size_t bytes_read;
        while ((bytes_read = fread(file_content, 1, BUFFER_SIZE, file)) > 0) {
            send(socket, file_content, bytes_read, 0);
        }
        fclose(file);

    } else {
        printf("❌ File not found: %s\n", real_file_path);
        FILE *file_404 = fopen("./www/error_404.html", "rb");  

        if (file_404 != NULL) {
            // Calculate the size of the 404 error file
            fseek(file_404, 0, SEEK_END);
            long file_size = ftell(file_404);
            fseek(file_404, 0, SEEK_SET);

            char http_header[BUFFER_SIZE] = {0};
            snprintf(http_header, sizeof(http_header), 
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: %ld\r\n\r\n", 
                     file_size);

            send(socket, http_header, strlen(http_header), 0);

            char file_content[BUFFER_SIZE] = {0};
            size_t bytes_read;
            while ((bytes_read = fread(file_content, 1, BUFFER_SIZE, file_404)) > 0) {
                send(socket, file_content, bytes_read, 0);
            }
            fclose(file_404);
        } else {
            // Hard emergency fallback (with manual Content-Length: 22 bytes)
            const char *not_found_response = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 22\r\n\r\n"
                "<h1>404 Not Found</h1>";
            send(socket, not_found_response, strlen(not_found_response), 0);
        }
    }
}