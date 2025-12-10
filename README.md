# CryptStream - Multi-Process File Encryption Engine

A high-performance CLI-based file encryption/decryption engine built with C++17, featuring parallel processing through a producer-consumer architecture with lazy process pool management.

## Features

- **Multi-Process Architecture**: Lazy process pool with producer-consumer pattern
- **Shared Memory IPC**: True memory sharing using mmap with circular task queue
- **Lock-Safe Synchronization**: Semaphores and mutex for thread-safe operations
- **High Performance**: 250% speedup on files >400KB compared to single-threaded
- **Modern C++17**: Leveraging std::move for efficient resource management
- **Benchmarking Suite**: Compare single-threaded vs multi-process performance

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                Main Process                                 │
│  ┌──────────────┐         ┌─────────────────────┐                           │
│  │  CLI Parser  │────────▶│    Task Producer    │                           │
│  └──────────────┘         └─────────────────────┘                           │
│                                      │                                      │
│                                      ▼                                      │
│                           ┌──────────────────────┐                          │
│                           │  Shared Memory       │                          │
│                           │  Circular Queue      │                          │
│                           │  (mmap)              │                          │
│                           └──────────────────────┘                          │
│                                      │                                      │
│                    ┌─────────────────┼─────────────────┐                    │
│                    ▼                 ▼                 ▼                    │
│            ┌──────────────┐  ┌──────────────┐  ┌──────────────┐             │
│            │ Child Proc 1 │  │ Child Proc 2 │  │ Child Proc N │             │
│            │  Consumer    │  │  Consumer    │  │  Consumer    │             │
│            └──────────────┘  └──────────────┘  └──────────────┘             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Building

```bash
make all
```

## Usage

```bash
# Encrypt a file
./cryptstream encrypt input.txt output.enc --key mykey

# Decrypt a file
./cryptstream decrypt output.enc decrypted.txt --key mykey

# Benchmark
./cryptstream benchmark --file testfile.dat --processes 4
```

## Performance

- Files >400KB: ~250% speedup with multi-process
- Files <5KB: Single-threaded performs better (overhead dominates)
- Optimal process count: 4-8 (depends on CPU cores)

## Requirements

- C++17 compatible compiler (g++ 7+ or clang++ 5+)
- POSIX-compliant system (Linux, macOS)
- Make build system

## License

MIT License
