#ifndef ROUTER_H
#define ROUTER_H

#include "request.h"

void route(Request *request, char *out_path, size_t out_path_size);

#endif