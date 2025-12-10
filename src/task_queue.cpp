#include "task_queue.hpp"
#include <stdexcept>

namespace cryptstream {

TaskQueue::TaskQueue(SharedMemory& shm, bool initialize)
    : data_(static_cast<QueueData*>(shm.get())),
      mutex_(&data_->mutex, initialize) {
    
    if (initialize) {
        data_->head = 0;
        data_->tail = 0;
        data_->count = 0;
        data_->shutdown = false;
    }
}

bool TaskQueue::enqueue(const Task& task) {
    mutex_.lock();
    
    if (data_->shutdown) {
        mutex_.unlock();
        return false;
    }
    
    if (data_->count >= MAX_TASKS) {
        mutex_.unlock();
        return false;  // Queue full
    }
    
    // Add task to tail
    data_->tasks[data_->tail] = task;
    data_->tail = (data_->tail + 1) % MAX_TASKS;
    data_->count++;
    
    mutex_.unlock();
    return true;
}

bool TaskQueue::dequeue(Task& task) {
    mutex_.lock();
    
    if (data_->count == 0) {
        bool shutdown = data_->shutdown;
        mutex_.unlock();
        return !shutdown;  // Return false only if shutdown
    }
    
    // Remove task from head
    task = data_->tasks[data_->head];
    data_->head = (data_->head + 1) % MAX_TASKS;
    data_->count--;
    
    mutex_.unlock();
    return true;
}

bool TaskQueue::is_empty() const {
    return data_->count == 0;
}

bool TaskQueue::is_full() const {
    return data_->count >= MAX_TASKS;
}

size_t TaskQueue::size() const {
    return data_->count;
}

void TaskQueue::signal_shutdown() {
    mutex_.lock();
    data_->shutdown = true;
    mutex_.unlock();
}

bool TaskQueue::is_shutdown() const {
    return data_->shutdown;
}

} // namespace cryptstream
