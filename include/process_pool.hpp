#ifndef CRYPTSTREAM_PROCESS_POOL_HPP
#define CRYPTSTREAM_PROCESS_POOL_HPP

#include "task_queue.hpp"
#include "shared_memory.hpp"
#include <vector>
#include <sys/types.h>
#include <unistd.h>

namespace cryptstream {

/**
 * Lazy process pool for parallel task execution
 * Creates child processes on-demand and manages their lifecycle
 */
class ProcessPool {
public:
    ProcessPool(size_t num_processes, TaskQueue& queue, 
                Semaphore& task_sem, Semaphore& done_sem);
    ~ProcessPool();
    
    // Non-copyable
    ProcessPool(const ProcessPool&) = delete;
    ProcessPool& operator=(const ProcessPool&) = delete;
    
    // Start worker processes
    void start();
    
    // Wait for all workers to complete
    void wait_all();
    
    // Terminate all workers
    void terminate();
    
    // Get number of active workers
    size_t active_workers() const { return worker_pids_.size(); }
    
private:
    size_t num_processes_;
    TaskQueue& queue_;
    Semaphore& task_sem_;
    Semaphore& done_sem_;
    std::vector<pid_t> worker_pids_;
    bool started_;
    
    // Worker process main loop
    static void worker_loop(int worker_id, TaskQueue& queue, 
                           Semaphore& task_sem, Semaphore& done_sem);
};

} // namespace cryptstream

#endif // CRYPTSTREAM_PROCESS_POOL_HPP
