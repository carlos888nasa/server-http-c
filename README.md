# C-Web-Server (Built from Scratch)

![Language](https://img.shields.io/badge/language-C-blue.svg)
![Standard](https://img.shields.io/badge/standard-C11-green.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

A low-level HTTP/1.1 server written in pure C, focused on performance-oriented design, using the Berkeley Sockets API.

**No external networking libraries. No shortcuts.**

## 📖 About The Project

The main goal of this project is to demystify the inner workings of web servers like Nginx or Apache. By building it from scratch, I aim to gain a deep, low-level understanding of:
* **TCP/IP Protocols:** Handling connections, packet buffering, and data streams manually.
* **Unix Systems Programming:** Manipulating File Descriptors, Signals, and System Calls.
* **Concurrency Models:** Implementing a **Thread Pool** to handle multiple clients simultaneously without the overhead of forking processes.
* **Memory Management:** Strict control over the Heap and Stack to prevent memory leaks in a long-running service.

## 🚀 Key Features (Planned & Implemented)

This project is evolving from a basic socket listener to a fully functional web server.

- [x] **Core Networking:** TCP Socket creation, binding, and listening (IPv4).
- [x] **Modular Architecture:** Separation of concerns (Server logic, HTTP protocol, File handling).
- [x] HTTP Parser: Basic HTTP/1.1 request parsing (extracting methods, paths, and headers).
- [ ] **Concurrency (Thread Pool):** A pre-allocated pool of worker threads ("Boss-Worker" model) to handle high-traffic loads efficiently.
- [x] **Static File Serving:** Serving HTML, CSS, JS, and images from the disk.
- [ ] **MIME Type Support:** Dynamic content-type headers based on file extensions.
- [x] **Error Handling:** robust 404 (Not Found) and 500 (Internal Server Error) pages.
- [x] **HTTP Router:** URL parsing to direct traffic between files and APIs.
- [x] **JSON API Endpoint:** Serving dynamic data for backend applications.
- [ ] **Security (Path Traversal):** Blocking malicious requests trying to access root files via `../`.
- [ ] **POST Processing:** Semantic handling of form data and API payloads (creating/updating resources).
- [ ] **Custom Logger:** Real-time access logs with timestamps and status codes.

## 🛠️ Technical Architecture

(Note: The server is currently running on a robust single-threaded event loop. The following multithreaded architecture is actively under development).

The server follows a **Boss-Worker Multithreading Pattern**:

1.  **Main Thread (The Boss):** Listens on port `8080`. It accepts incoming connections (`accept()`) and pushes the new socket descriptor into a **Task Queue**.
2.  **Worker Threads:** A fixed number of threads (e.g., 10) sleep until a task arrives. They wake up, pick the socket from the queue, process the HTTP request, send the response, and close the connection.
3.  **Synchronization:** Uses `mutex` and `condition variables` (from `<pthread.h>`) to prevent race conditions when accessing the shared queue.

## 📦 Getting Started

### Prerequisites
* **OS:** Linux (Ubuntu/Debian) or Windows via WSL2.
* **Compiler:** GCC.
* **Build Tool:** GNU Make.

## 🌐 Supported Endpoints

| Method | Endpoint      | Description                                      | Content-Type       |
|--------|---------------|--------------------------------------------------|--------------------|
| GET    | `/`           | Redirects to `index.html` (Homepage)             | `text/html`        |
| GET    | `/api`        | Returns backend server status data               | `application/json` |
| GET    | `/*`          | Dynamic file router (fetches matching file)      | `text/html`        |
| GET    | `/invalid`    | Returns custom Dark Mode 404 Page                | `text/html`        |

## 🧪 Testing & Validation

- **Manual Testing:** Endpoints verified via browser and `curl`.
- **Memory Safety:** `Valgrind` verification planned to ensure zero memory leaks.
- **Stress Testing:** Load testing (using `ab` or `wrk`) planned for the upcoming multithreaded phase.

### Installation & Usage

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/carlos888nasa/server-http-c.git
    cd server-http-c
    ```

2.  **Compile the source code:**
    The project uses a Makefile for automated builds.
    ```bash
    make
    ```

3.  **Run the server:**
    Pass the desired port as an argument.
    ```bash
    ./bin/server 8080
    ```

4.  **Test it:**
    Open your browser and navigate to `http://localhost:8080`, or test the JSON API directly from your terminal using `curl`:

    ```bash
    curl -i http://localhost:8080/api
    ```
    *(Expected output: HTTP/1.1 200 OK with the server status in JSON format).*

## 📂 Project Structure

```text
.
├── src/            # Source code (.c)
├── include/        # Header files (.h)
├── bin/            # Compiled executables (ignored by git)
├── data/           # Private Server Data (JSON API files)
├── www/            # Static website files (HTML, CSS)
├── Makefile        # Build configuration
└── README.md       # Documentation

```

## 🤝 Contributing
This is an educational project, but suggestions and optimizations are welcome. Feel free to open an issue.

## 📝 License
Distributed under the MIT License.