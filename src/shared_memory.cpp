#include "shared_memory.hpp"
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>
#include <errno.h>

namespace cryptstream {

// ============================================================================
// SharedMemory Implementation
// ============================================================================

SharedMemory::SharedMemory(const std::string& name, size_t size, bool create)
    : name_(name), ptr_(nullptr), size_(size), fd_(-1), owner_(create) {
    
    int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
    mode_t mode = S_IRUSR | S_IWUSR;
    
    // Open or create shared memory object
    fd_ = shm_open(name_.c_str(), flags, mode);
    if (fd_ == -1) {
        throw std::runtime_error("Failed to open shared memory: " + 
                                 std::string(strerror(errno)));
    }
    
    // Set size if creating
    if (create) {
        if (ftruncate(fd_, size_) == -1) {
            close(fd_);
            shm_unlink(name_.c_str());
            throw std::runtime_error("Failed to set shared memory size: " + 
                                     std::string(strerror(errno)));
        }
    }
    
    // Map shared memory into process address space
    ptr_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (ptr_ == MAP_FAILED) {
        close(fd_);
        if (create) {
            shm_unlink(name_.c_str());
        }
        throw std::runtime_error("Failed to map shared memory: " + 
                                 std::string(strerror(errno)));
    }
    
    // Initialize memory to zero if creating
    if (create) {
        std::memset(ptr_, 0, size_);
    }
}

SharedMemory::~SharedMemory() {
    if (ptr_ != nullptr && ptr_ != MAP_FAILED) {
        munmap(ptr_, size_);
    }
    if (fd_ != -1) {
        close(fd_);
    }
}

void SharedMemory::unlink() {
    if (owner_) {
        shm_unlink(name_.c_str());
    }
}

// ============================================================================
// Semaphore Implementation
// ============================================================================

Semaphore::Semaphore(const std::string& name, unsigned int initial_value, bool create)
    : name_(name), sem_(nullptr), owner_(create) {
    
    if (create) {
        // Create new semaphore
        sem_ = sem_open(name_.c_str(), O_CREAT | O_EXCL, 0644, initial_value);
        if (sem_ == SEM_FAILED) {
            // If already exists, unlink and try again
            sem_unlink(name_.c_str());
            sem_ = sem_open(name_.c_str(), O_CREAT | O_EXCL, 0644, initial_value);
        }
    } else {
        // Open existing semaphore
        sem_ = sem_open(name_.c_str(), 0);
    }
    
    if (sem_ == SEM_FAILED) {
        throw std::runtime_error("Failed to open semaphore: " + 
                                 std::string(strerror(errno)));
    }
}

Semaphore::~Semaphore() {
    if (sem_ != nullptr && sem_ != SEM_FAILED) {
        sem_close(sem_);
    }
}

void Semaphore::wait() {
    if (sem_wait(sem_) == -1) {
        throw std::runtime_error("Semaphore wait failed: " + 
                                 std::string(strerror(errno)));
    }
}

void Semaphore::post() {
    if (sem_post(sem_) == -1) {
        throw std::runtime_error("Semaphore post failed: " + 
                                 std::string(strerror(errno)));
    }
}

bool Semaphore::try_wait() {
    return sem_trywait(sem_) == 0;
}

void Semaphore::unlink() {
    if (owner_) {
        sem_unlink(name_.c_str());
    }
}

// ============================================================================
// SharedMutex Implementation
// ============================================================================

SharedMutex::SharedMutex(pthread_mutex_t* mutex, bool initialize)
    : mutex_(mutex) {
    
    if (initialize) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        
        // Set mutex to be shared between processes
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        
        // Initialize the mutex
        pthread_mutex_init(mutex_, &attr);
        
        pthread_mutexattr_destroy(&attr);
    }
}

void SharedMutex::lock() {
    if (pthread_mutex_lock(mutex_) != 0) {
        throw std::runtime_error("Mutex lock failed");
    }
}

void SharedMutex::unlock() {
    if (pthread_mutex_unlock(mutex_) != 0) {
        throw std::runtime_error("Mutex unlock failed");
    }
}

bool SharedMutex::try_lock() {
    return pthread_mutex_trylock(mutex_) == 0;
}

} // namespace cryptstream
