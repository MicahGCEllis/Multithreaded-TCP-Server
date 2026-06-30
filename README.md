# Planet 9: Fiber TCP Server (Repo 1)

A bare-metal, ultra-high-performance HTTP TCP server built in C++20. 

This repository serves as the foundational network engine for the Planet 9 ecosystem. It aggressively bypasses standard thread-per-connection models and third-party libraries (like Boost.Asio or libuv). Instead, it relies on raw OS-level asynchronous I/O and custom user-space context switching (fibers) to handle massive concurrent traffic with zero-cost abstractions.

## Core Features
* **Custom Fiber Scheduler:** Hand-written Assembly (`x86_64`) for cooperative user-space multitasking. Thread execution pauses and resumes at the register level, eliminating OS context-switch overhead.
* **Proactive Asynchronous I/O:** Utilizes Windows IOCP (with scalable architecture ready for Linux `epoll` and macOS `kqueue`). Sockets are pre-posted to the kernel for instant connection handoffs.
* **Strict CPU Affinity:** Worker threads are pinned directly to physical CPU cores, maximizing L1/L2 cache locality and preventing the OS scheduler from scrambling resources.
* **Lock-Free Telemetry Pipeline:** A dedicated asynchronous logger and telemetry bridge captures traffic metrics, CPU saturation, and system events in real-time to CSV, completely unblocking the main execution threads.
* **Hardened Security Layer:** An uncompromising HTTP parser and URL decoder that sanitizes inputs and blocks directory traversal attacks (`../`) before they reach the OS file system.

## Project Structure
* `/src` - Core engine logic (Thread Pool, TcpServer, Connection Handler).
* `/src/asm` - Architecture-specific assembly files for register state saving and stack pointer alignment.
* `/src/io` - OS-specific implementations for Async I/O and CPU Affinity.
* `/tests` - Comprehensive C++ unit tests mathematically proving ABI boundaries, concurrency safety, and loopback network handling.

## Quick Start
1. Ensure you have a C++20 compatible compiler and CMake installed.
2. Clone the repository.
3. Generate the build files:
   ```bash
   cmake -B build