# Fiber TCP Server Architecture

## Overview
This repository contains the core networking engine for the system. It is designed to maximize hardware efficiency by bypassing standard thread-per-connection models. Instead, it utilizes highly optimized OS-level asynchronous I/O, strict CPU core affinity, and user-space context switching (fibers) to handle thousands of concurrent connections with minimal overhead.

## Core Systems

### 1. The Network & I/O Layer (The Front Door)
* **Mechanism:** Utilizes advanced OS-specific event notification APIs (IOCP on Windows, Epoll on Linux, Kqueue on macOS).
* **Purpose:** Prevents blocking. The server registers open connections with the kernel and is only notified when a socket is actively ready to be read from or written to, eliminating wasted CPU cycles polling idle connections.

### 2. The Worker Threads (The Engine Room)
* **Mechanism:** A fixed-size thread pool statically mapped to the physical cores of the CPU.
* **Purpose:** By locking one worker thread strictly to one core (CPU Affinity), the operating system is prevented from shuffling threads across the processor. This maximizes L1/L2 cache locality and eliminates the heavy overhead of OS thread context switching. 

### 3. The Fiber Scheduler (The Jugglers)
* **Mechanism:** User-space cooperative multitasking via custom assembly (`switch_x64.S`).
* **Purpose:** When a task needs to wait for network data, freezing the OS worker thread is unacceptably slow. Instead, the current task voluntarily saves its own execution state (registers and stack pointer) and yields. The worker thread instantly swaps in a new, ready fiber. When the original network I/O completes, the suspended fiber is seamlessly rotated back in to resume execution.

### 4. Application Logic & Security (The Checkpoint)
* **Mechanism:** HTTP parsing and strict directory traversal validation.
* **Purpose:** Once a fiber reconstructs a complete client request, it parses the payload. Before any file is served from the local directory, the path is sanitized and verified to ensure malicious clients cannot escape the designated `public/` directory to read internal system files.

### 5. Telemetry & Logging (The Dashboard)
* **Mechanism:** Lock-free concurrent queues and shared memory blocks.
* **Purpose:** High-performance workers cannot block to perform disk I/O. Logs and metrics are immediately tossed into an asynchronous queue and processed by a dedicated background thread. This keeps the primary workers unblocked while maintaining live performance metrics and persistent server logs.

## Request Lifecycle
1.  **Connection:** A client initiates a TCP connection on port `8080`.
2.  **Acceptance:** The I/O layer accepts the socket and registers it with the OS event queue.
3.  **Assignment:** A worker thread pulls the new event and assigns it to a Fiber.
4.  **Processing:** The Fiber parses the client's information. If the data stream is incomplete, the Fiber yields back to the scheduler.
5.  **Validation:** Once the request is fully read, the requested file path is verified by the security module.
6.  **Response:** The server returns the standard HTML page or requested static file.
7.  **Telemetry:** The action is logged asynchronously, and the Fiber is recycled for the next connection.