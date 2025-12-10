#ifndef CRYPTSTREAM_SHARED_MEMORY_HPP
#define CRYPTSTREAM_SHARED_MEMORY_HPP

#include <string>
#include <cstddef>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

namespace cryptstream {

/**
 * Shared memory region using mmap
 * Provides true memory sharing across processes (no copy-on-write)
 */
class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size, bool create = true);
    ~SharedMemory();
    
    // Non-copyable
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    
    // Get pointer to shared memory
    void* get() const { return ptr_; }
    
    // Get size of shared memory
    size_t size() const { return size_; }
    
    // Unlink shared memory (call from parent process)
    void unlink();
    
private:
    std::string name_;
    void* ptr_;
    size_t size_;
    int fd_;
    bool owner_;
};

/**
 * POSIX semaphore wrapper for process synchronization
 */
class Semaphore {
public:
    Semaphore(const std::string& name, unsigned int initial_value, bool create = true);
    ~Semaphore();
    
    // Non-copyable
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    
    void wait();
    void post();
    bool try_wait();
    
    void unlink();
    
private:
    std::string name_;
    sem_t* sem_;
    bool owner_;
};

/**
 * POSIX mutex in shared memory for process synchronization
 */
class SharedMutex {
public:
    explicit SharedMutex(pthread_mutex_t* mutex, bool initialize = false);
    ~SharedMutex() = default;
    
    void lock();
    void unlock();
    bool try_lock();
    
private:
    pthread_mutex_t* mutex_;
};

} // namespace cryptstream

#endif // CRYPTSTREAM_SHARED_MEMORY_HPP
