#include <string.h>
#include <stdio.h>

#include "../include/router.h"

void route(Request *request, char *out_path) {
    
    if (strncmp(request->path, "/api", 4) != 0) {
        if (strcmp(request->path, "/") == 0) {
            strncpy(request->path, "/index.html", sizeof(request->path) - 1);
            request->path[sizeof(request->path)-1] = '\0';
        } else {
            char *last_slash = strrchr(request->path, '/');
            char *dot_after_slash = (last_slash) ? strchr(last_slash, '.') : NULL;
            // Si no hay punto, asumimos HTML
            if (!dot_after_slash) {
                strncat(request->path, ".html", sizeof(request->path) - strlen(request->path) - 1);
            }
        }
    }

    if (!strncmp(request->path, "/api", 4)) {
        if (!strcmp(request->path, "/api") || !strcmp(request->path, "/api/")) {
            snprintf(out_path, 512, "./data/status.json");
        } else {
            snprintf(out_path, 512, "./data%s", request->path + 4);
        }
    } else {
        snprintf(out_path, 512, "./www%s", request->path);
    }
    
    printf("DEBUG -> Route: '%s'\n", out_path);
}
