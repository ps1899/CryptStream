#ifndef CRYPTSTREAM_TASK_QUEUE_HPP
#define CRYPTSTREAM_TASK_QUEUE_HPP

#include "shared_memory.hpp"
#include <string>
#include <cstddef>
#include <cstring>

namespace cryptstream {

/**
 * Task structure for encryption/decryption operations
 * Stored directly in shared memory (no pointers!)
 */
struct Task {
    enum Type { ENCRYPT, DECRYPT, TERMINATE };
    
    Type type;
    char input_file[256];
    char output_file[256];
    char key[64];
    bool completed;
    int worker_id;
    
    Task() : type(TERMINATE), completed(false), worker_id(-1) {
        input_file[0] = '\0';
        output_file[0] = '\0';
        key[0] = '\0';
    }
    
    void set_input(const std::string& path) {
        std::strncpy(input_file, path.c_str(), sizeof(input_file) - 1);
        input_file[sizeof(input_file) - 1] = '\0';
    }
    
    void set_output(const std::string& path) {
        std::strncpy(output_file, path.c_str(), sizeof(output_file) - 1);
        output_file[sizeof(output_file) - 1] = '\0';
    }
    
    void set_key(const std::string& k) {
        std::strncpy(key, k.c_str(), sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
    }
};

/**
 * Circular task queue in shared memory
 * Thread-safe, lock-free for single producer/consumer
 * Uses array-based storage to avoid pointer issues in shared memory
 */
class TaskQueue {
public:
    static constexpr size_t MAX_TASKS = 128;
    
    /**
     * Shared memory layout for the queue
     */
    struct QueueData {
        pthread_mutex_t mutex;
        size_t head;
        size_t tail;
        size_t count;
        bool shutdown;
        Task tasks[MAX_TASKS];
    };
    
    TaskQueue(SharedMemory& shm, bool initialize = false);
    
    // Producer operations
    bool enqueue(const Task& task);
    
    // Consumer operations
    bool dequeue(Task& task);
    
    // Queue status
    bool is_empty() const;
    bool is_full() const;
    size_t size() const;
    
    // Shutdown signal
    void signal_shutdown();
    bool is_shutdown() const;
    
private:
    QueueData* data_;
    SharedMutex mutex_;
};

} // namespace cryptstream

#endif // CRYPTSTREAM_TASK_QUEUE_HPP
