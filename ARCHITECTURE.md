# CryptStream Architecture

## Overview

CryptStream is a high-performance, multi-process file encryption/decryption engine built with C++17. It demonstrates advanced systems programming concepts including inter-process communication, shared memory management, and producer-consumer patterns.

## Core Components

### 1. Encryption Engine (`crypto.hpp/cpp`)
- **Algorithm**: XOR-based encryption (demonstration purposes)
- **Key Expansion**: 256-byte expanded key from user input
- **In-place Processing**: Modifies data directly to minimize memory overhead
- **Symmetric**: Same operation for encryption and decryption

### 2. Shared Memory Management (`shared_memory.hpp/cpp`)

#### SharedMemory Class
- Uses POSIX `shm_open()` and `mmap()` for true memory sharing
- Avoids copy-on-write semantics of `fork()`
- Automatically handles cleanup via RAII
- Memory layout is fixed at creation time

#### Semaphore Class
- POSIX named semaphores for process synchronization
- Used for signaling task availability and completion
- Supports wait(), post(), and try_wait() operations

#### SharedMutex Class
- Process-shared pthread mutex
- Protects critical sections in shared memory
- Configured with `PTHREAD_PROCESS_SHARED` attribute

### 3. Task Queue (`task_queue.hpp/cpp`)

#### Design Principles
- **Circular Buffer**: Fixed-size array (128 tasks) in shared memory
- **No Pointers**: All data stored inline to avoid pointer misalignment
- **Lock-Safe**: Protected by process-shared mutex
- **Producer-Consumer**: Single producer (main), multiple consumers (workers)

#### Task Structure
```cpp
struct Task {
    Type type;              // ENCRYPT, DECRYPT, TERMINATE
    char input_file[256];   // Fixed-size arrays
    char output_file[256];
    char key[64];
    bool completed;
    int worker_id;
};
```

#### Queue Operations
- `enqueue()`: Add task to tail (producer)
- `dequeue()`: Remove task from head (consumer)
- `signal_shutdown()`: Graceful termination signal

### 4. File Processor (`file_processor.hpp/cpp`)

#### std::move Semantics
- **Ownership Transfer**: File streams moved between functions
- **No Copying**: Avoids expensive deep copies of stream objects
- **RAII**: Automatic resource cleanup when streams go out of scope

```cpp
std::vector<uint8_t> read_file(std::ifstream&& input) {
    std::ifstream file = std::move(input);  // Transfer ownership
    // ... read operations ...
    // file automatically closed on return
}
```

### 5. Process Pool (`process_pool.hpp/cpp`)

#### Lazy Process Creation
- Processes created on-demand via `fork()`
- Each worker runs in separate address space
- Shared memory accessible to all processes

#### Worker Lifecycle
1. **Start**: Fork child process
2. **Loop**: Wait for tasks via semaphore
3. **Process**: Execute encryption/decryption
4. **Signal**: Post completion semaphore
5. **Terminate**: Exit on shutdown signal

#### Process Communication
```
Main Process                    Worker Processes
     │                               │
     ├─ enqueue(task) ──────────────▶│
     ├─ post(task_sem) ─────────────▶│
     │                               ├─ wait(task_sem)
     │                               ├─ dequeue(task)
     │                               ├─ process_file()
     │◀──────────────── post(done_sem)
     ├─ wait(done_sem)
```

## Producer-Consumer Architecture

### Synchronization Primitives

1. **Task Semaphore** (`task_sem`)
   - Signals task availability
   - Workers block on wait()
   - Producer posts after enqueue

2. **Done Semaphore** (`done_sem`)
   - Signals task completion
   - Producer blocks on wait()
   - Workers post after processing

3. **Queue Mutex** (in TaskQueue)
   - Protects queue data structure
   - Ensures atomic enqueue/dequeue

### Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      Main Process                            │
│                                                              │
│  1. Create shared memory                                     │
│  2. Initialize task queue                                    │
│  3. Create semaphores                                        │
│  4. Fork worker processes                                    │
│  5. Enqueue tasks                                           │
│  6. Post task_sem (signal workers)                          │
│  7. Wait on done_sem (block until complete)                 │
│  8. Signal shutdown                                          │
│  9. Wait for workers to exit                                │
│  10. Cleanup resources                                       │
└─────────────────────────────────────────────────────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │   Shared Memory        │
              │   ┌──────────────────┐ │
              │   │  Task Queue      │ │
              │   │  - head          │ │
              │   │  - tail          │ │
              │   │  - count         │ │
              │   │  - tasks[128]    │ │
              │   │  - mutex         │ │
              │   └──────────────────┘ │
              └────────────────────────┘
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
    ┌─────────┐      ┌─────────┐      ┌─────────┐
    │Worker 0 │      │Worker 1 │      │Worker N │
    │         │      │         │      │         │
    │ Loop:   │      │ Loop:   │      │ Loop:   │
    │ 1. wait │      │ 1. wait │      │ 1. wait │
    │ 2. deq  │      │ 2. deq  │      │ 2. deq  │
    │ 3. proc │      │ 3. proc │      │ 3. proc │
    │ 4. post │      │ 4. post │      │ 4. post │
    └─────────┘      └─────────┘      └─────────┘
```

## Memory Layout

### Shared Memory Region
```
┌─────────────────────────────────────┐
│ pthread_mutex_t mutex               │  Process-shared mutex
├─────────────────────────────────────┤
│ size_t head                         │  Queue head index
├─────────────────────────────────────┤
│ size_t tail                         │  Queue tail index
├─────────────────────────────────────┤
│ size_t count                        │  Current task count
├─────────────────────────────────────┤
│ bool shutdown                       │  Shutdown flag
├─────────────────────────────────────┤
│ Task tasks[128]                     │  Circular task array
│   - Task 0                          │
│   - Task 1                          │
│   - ...                             │
│   - Task 127                        │
└─────────────────────────────────────┘
```

### Why No Pointers?
- Shared memory mapped at different virtual addresses in each process
- Pointers would be invalid across process boundaries
- Solution: Use fixed-size arrays and offsets

## Performance Characteristics

### Single-Threaded Mode
- **Advantages**: No overhead, simple execution
- **Best For**: Files < 5KB
- **Overhead**: None

### Multi-Process Mode
- **Advantages**: Parallel execution, CPU utilization
- **Best For**: Files > 400KB
- **Overhead**: Process creation, IPC, synchronization
- **Speedup**: ~250% for large files (4 workers)

### Overhead Analysis
```
Total Time = Process_Creation + Task_Distribution + Processing + Synchronization

For small files: Overhead > Processing_Time_Saved
For large files: Processing_Time_Saved > Overhead
```

## Key Design Decisions

### 1. Why Processes Instead of Threads?
- Demonstrates true multi-process IPC
- Isolation between workers
- Educational value for systems programming

### 2. Why mmap Instead of Message Queues?
- Direct memory access (faster)
- Flexible data structures
- Demonstrates shared memory concepts

### 3. Why Circular Queue?
- Fixed memory footprint
- Cache-friendly access pattern
- Simple index arithmetic

### 4. Why std::move for File Streams?
- Demonstrates C++11/17 move semantics
- Avoids copying file descriptors
- Clear ownership transfer

## Error Handling

### Shared Memory Errors
- Failed `shm_open()`: Throw runtime_error
- Failed `mmap()`: Cleanup and throw
- Automatic cleanup via RAII destructors

### Process Errors
- Failed `fork()`: Log error, continue with fewer workers
- Worker crash: Detected by parent via `waitpid()`
- Graceful shutdown on SIGTERM

### File I/O Errors
- File not found: Return false, log error
- Permission denied: Return false, log error
- Disk full: Exception propagated to caller

## Future Enhancements

1. **Batch Processing**: Multiple files in single run
2. **AES Encryption**: Replace XOR with industry-standard algorithm
3. **Progress Reporting**: Real-time status updates
4. **Dynamic Pool Sizing**: Adjust workers based on load
5. **Network Support**: Distributed processing across machines

## References

- POSIX Shared Memory: `man shm_overview`
- POSIX Semaphores: `man sem_overview`
- C++17 Move Semantics: cppreference.com
- Producer-Consumer Pattern: Classic IPC problem
