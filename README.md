# C-Web-Server (Built from Scratch)

![Language](https://img.shields.io/badge/language-C-blue.svg)
![Standard](https://img.shields.io/badge/standard-C11-green.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

A low-level HTTP/1.1 server written in pure C, built on top of the Berkeley Sockets API with a multithreaded Boss-Worker architecture.

**No external networking libraries. No shortcuts.**

---

## About The Project

The main goal of this project is to demystify the inner workings of web servers like Nginx or Apache. By building it from scratch, I gain a deep, low-level understanding of:

- **TCP/IP Protocols:** Handling connections, packet buffering, and data streams manually.
- **Unix Systems Programming:** Manipulating file descriptors, signals, and system calls directly.
- **Concurrency Models:** A pre-allocated thread pool to handle multiple clients simultaneously without the overhead of forking processes.
- **Memory Management:** Strict control over heap and stack to prevent memory leaks in a long-running service.

---

## Key Features

- [x] **Core Networking:** TCP socket creation, binding, and listening (IPv4).
- [x] **Modular Architecture:** Clean separation of concerns across dedicated modules (server, request, router, response, mime, threadpool).
- [x] **HTTP/1.1 Parser:** Request parsing — method, path, query string stripping, and header extraction.
- [x] **Thread Pool (Boss-Worker):** Pre-allocated worker threads using `pthread`, `mutex`, and condition variables. Thread count auto-detected from CPU cores via `sysconf(_SC_NPROCESSORS_ONLN)`.
- [x] **Static File Serving:** Serving HTML, CSS, JS, JSON, PNG, JPG, and GIF from disk.
- [x] **MIME Type Detection:** Dynamic `Content-Type` headers based on file extension.
- [x] **Content-Length Headers:** All responses include accurate `Content-Length` for correct HTTP/1.1 compliance.
- [x] **Error Handling:** Custom 404 page with emergency fallback. 503 response when the thread pool queue is full.
- [x] **HTTP Router:** URL routing between static files (`/www`) and JSON API endpoints (`/data`).
- [x] **Security:** Path traversal (`../`) blocking. `SO_REUSEADDR` to prevent port lock after restart. 5-second `recv()` timeout to prevent slow-client DoS.
- [x] **Graceful Shutdown:** `SIGINT` handler (`Ctrl+C`) closes the socket cleanly using only async-signal-safe calls (`write`, `_exit`).

---

## Architecture

The server follows a **Boss-Worker Multithreading Pattern**:

1. **Main Thread (The Boss):** Listens on the specified port. Accepts incoming TCP connections and pushes socket descriptors into a circular task queue.
2. **Worker Threads:** A pool of N threads (one per CPU core) sleep on a condition variable until a task arrives. Each worker picks a socket, parses the HTTP request, routes it, sends the response, and closes the connection.
3. **Synchronization:** A `pthread_mutex_t` protects the shared queue. A `pthread_cond_t` signals workers when work is available. The queue is a fixed-size circular buffer — if full, the server responds with `503 Service Unavailable` and drops the connection cleanly.

```
                  ┌─────────────────────────────────────┐
  TCP Client ───► │  accept()  →  Task Queue (circular) │  Main Thread
                  └────────────────┬────────────────────┘
                                   │ pthread_cond_signal
               ┌───────────────────┼───────────────────┐
               ▼                   ▼                   ▼
          Worker 0           Worker 1  ...        Worker N
        parse_request()    parse_request()      parse_request()
          route()            route()              route()
        send_response()    send_response()      send_response()
          close()            close()              close()
```

---

## Project Structure

```
server-http-c/
├── src/
│   ├── main.c          # Entry point — argument validation and server boot
│   ├── server.c        # Socket setup, accept loop, signal handling
│   ├── request.c       # HTTP request parsing and validation
│   ├── router.c        # URL-to-filepath routing logic
│   ├── response.c      # HTTP response building and file sending
│   ├── mime.c          # MIME type detection by file extension
│   └── threadpool.c    # Boss-Worker thread pool implementation
├── include/
│   ├── server.h
│   ├── request.h       # Defines Request struct and BUFFER_SIZE
│   ├── router.h
│   ├── response.h
│   ├── mime.h
│   └── threadpool.h    # Defines ThreadPoolManager struct
├── www/                # Static website files (HTML, CSS, JS)
├── data/               # JSON API files
├── bin/                # Compiled binary (git-ignored)
├── Makefile
└── README.md
```

---

## Getting Started

### Prerequisites

- **OS:** Linux (Ubuntu/Debian) or Windows via WSL2
- **Compiler:** GCC with C11 support
- **Build Tool:** GNU Make

### Build & Run

```bash
# Clone
git clone https://github.com/carlos888nasa/server-http-c.git
cd server-http-c

# Compile
make

# Run (pass port as argument)
./bin/server 8080
```

### Test

```bash
# Browser
open http://localhost:8080

# curl
curl -i http://localhost:8080/api

# Load test (requires wrk)
wrk -t4 -c100 -d10s http://localhost:8080/
```

---

## Supported Endpoints

| Method | Endpoint   | Description                              | Content-Type       |
|--------|------------|------------------------------------------|--------------------|
| GET    | `/`        | Serves `index.html`                      | `text/html`        |
| GET    | `/api`     | Returns server status JSON               | `application/json` |
| GET    | `/*`       | Dynamic file router (serves matching file) | varies           |
| GET    | `/invalid` | Returns custom 404 page                  | `text/html`        |

---

## Roadmap

- [ ] **TLS/HTTPS** via OpenSSL (`SSL_read` / `SSL_write` socket wrapping)
- [ ] **HTTP/1.1 Keep-Alive** — reuse TCP connections across multiple requests
- [ ] **POST request handling** — parse request body and form data
- [ ] **Custom logger** — timestamped access logs with method, path, status, and latency
- [ ] **IPv6 support** — dual-stack `AF_INET6` socket
- [ ] **CI pipeline** — GitHub Actions: compile with `-fsanitize=address`, run curl tests
- [ ] **Valgrind** — zero memory leaks verified and enforced in CI

---

## License

Distributed under the MIT License.