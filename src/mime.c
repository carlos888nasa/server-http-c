#include <string.h>

#include "../include/mime.h"

typedef struct{
    const char *ext;
    const char *type;
}MimeEntry;

static const MimeEntry mime_table[] = {
    { ".html",  "text/html" },
    { ".css",   "text/css" },
    { ".js",    "application/javascript" },
    { ".json",  "application/json" },
    { ".png",   "image/png" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".gif",   "image/gif" },
    { ".svg",   "image/svg+xml" },
    { ".webp",  "image/webp" },
    { ".ico",   "image/x-icon" },
    { ".woff",  "font/woff" },
    { ".woff2", "font/woff2" },
    { ".pdf",   "application/pdf" },
    { ".mp4",   "video/mp4" },
    { ".webm",  "video/webm" },
    { ".mp3",   "audio/mpeg" },
    { ".xml",   "application/xml" },
    { ".csv",   "text/csv" },
    { ".txt",   "text/plain" },
    {NULL, NULL}
};

// Function to determine content type based on file extension
const char* get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream"; // Default binary type

    for(int i = 0; mime_table[i].ext; i++ ){
        if(strcasecmp(ext, mime_table[i].ext) == 0)
        return mime_table[i].type;
    }

    return "application/octet-stream"; // Default binary type
}