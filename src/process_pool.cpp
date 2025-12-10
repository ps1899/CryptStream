#include "process_pool.hpp"
#include "file_processor.hpp"
#include <iostream>
#include <sys/wait.h>
#include <signal.h>
#include <chrono>
#include <thread>

namespace cryptstream {

ProcessPool::ProcessPool(size_t num_processes, TaskQueue& queue,
                         Semaphore& task_sem, Semaphore& done_sem)
    : num_processes_(num_processes),
      queue_(queue),
      task_sem_(task_sem),
      done_sem_(done_sem),
      started_(false) {
}

ProcessPool::~ProcessPool() {
    if (started_) {
        terminate();
    }
}

void ProcessPool::start() {
    if (started_) {
        return;
    }
    
    for (size_t i = 0; i < num_processes_; ++i) {
        pid_t pid = fork();
        
        if (pid < 0) {
            std::cerr << "Failed to fork worker process " << i << std::endl;
            continue;
        }
        
        if (pid == 0) {
            // Child process
            worker_loop(i, queue_, task_sem_, done_sem_);
            exit(0);  // Worker exits when done
        } else {
            // Parent process
            worker_pids_.push_back(pid);
            std::cout << "Started worker process " << i << " (PID: " << pid << ")" << std::endl;
        }
    }
    
    started_ = true;
}

void ProcessPool::wait_all() {
    for (pid_t pid : worker_pids_) {
        int status;
        waitpid(pid, &status, 0);
    }
    worker_pids_.clear();
    started_ = false;
}

void ProcessPool::terminate() {
    // Send termination signal to all workers
    for (pid_t pid : worker_pids_) {
        kill(pid, SIGTERM);
    }
    
    // Wait for workers to terminate
    wait_all();
}

void ProcessPool::worker_loop(int worker_id, TaskQueue& queue,
                              Semaphore& task_sem, Semaphore& done_sem) {
    std::cout << "Worker " << worker_id << " started" << std::endl;
    
    while (true) {
        // Wait for task availability
        task_sem.wait();
        
        // Check if shutdown
        if (queue.is_shutdown()) {
            std::cout << "Worker " << worker_id << " shutting down" << std::endl;
            break;
        }
        
        // Dequeue task
        Task task;
        if (!queue.dequeue(task)) {
            if (queue.is_shutdown()) {
                break;
            }
            continue;
        }
        
        // Check for termination task
        if (task.type == Task::TERMINATE) {
            std::cout << "Worker " << worker_id << " received termination signal" << std::endl;
            break;
        }
        
        // Process the task
        std::cout << "Worker " << worker_id << " processing: " 
                  << task.input_file << " -> " << task.output_file << std::endl;
        
        bool success = FileProcessor::process_file(task);
        
        if (success) {
            std::cout << "Worker " << worker_id << " completed task successfully" << std::endl;
        } else {
            std::cerr << "Worker " << worker_id << " failed to process task" << std::endl;
        }
        
        // Signal task completion
        done_sem.post();
    }
    
    std::cout << "Worker " << worker_id << " exiting" << std::endl;
}

} // namespace cryptstream
