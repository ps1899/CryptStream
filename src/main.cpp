#include "crypto.hpp"
#include "shared_memory.hpp"
#include "task_queue.hpp"
#include "process_pool.hpp"
#include "file_processor.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

using namespace cryptstream;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <command> [options]\n\n"
              << "Commands:\n"
              << "  encrypt <input> <output> --key <key> [--processes N]\n"
              << "  decrypt <input> <output> --key <key> [--processes N]\n"
              << "  batch <file_list> --key <key> [--processes N]\n\n"
              << "Options:\n"
              << "  --key <key>        Encryption/decryption key (required)\n"
              << "  --processes N      Number of worker processes (default: 4)\n\n"
              << "Examples:\n"
              << "  " << program_name << " encrypt input.txt output.enc --key mykey\n"
              << "  " << program_name << " decrypt output.enc decrypted.txt --key mykey\n"
              << "  " << program_name << " batch files.txt --key mykey --processes 8\n";
}

struct Config {
    std::string command;
    std::string input_file;
    std::string output_file;
    std::string key;
    size_t num_processes = 4;
    std::vector<std::pair<std::string, std::string>> file_pairs;
};

bool parse_args(int argc, char* argv[], Config& config) {
    if (argc < 2) {
        return false;
    }
    
    config.command = argv[1];
    
    if (config.command == "encrypt" || config.command == "decrypt") {
        if (argc < 4) {
            return false;
        }
        config.input_file = argv[2];
        config.output_file = argv[3];
        
        for (int i = 4; i < argc; ++i) {
            if (std::strcmp(argv[i], "--key") == 0 && i + 1 < argc) {
                config.key = argv[++i];
            } else if (std::strcmp(argv[i], "--processes") == 0 && i + 1 < argc) {
                config.num_processes = std::stoi(argv[++i]);
            }
        }
        
        return !config.key.empty();
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    Config config;
    
    if (!parse_args(argc, argv, config)) {
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        // Determine if we should use multi-process or single-threaded
        size_t file_size = FileProcessor::get_file_size(config.input_file);
        bool use_multiprocess = (file_size > 5000 && config.num_processes > 1);
        
        if (!use_multiprocess) {
            // Single-threaded processing for small files
            std::cout << "Using single-threaded processing (file size: " 
                      << file_size << " bytes)" << std::endl;
            
            Task task;
            task.type = (config.command == "encrypt") ? Task::ENCRYPT : Task::DECRYPT;
            task.set_input(config.input_file);
            task.set_output(config.output_file);
            task.set_key(config.key);
            
            if (FileProcessor::process_file(task)) {
                std::cout << "File processed successfully!" << std::endl;
                return 0;
            } else {
                std::cerr << "Failed to process file" << std::endl;
                return 1;
            }
        }
        
        // Multi-process processing
        std::cout << "Using multi-process processing with " << config.num_processes 
                  << " workers (file size: " << file_size << " bytes)" << std::endl;
        
        // Create shared memory for task queue
        size_t shm_size = sizeof(TaskQueue::QueueData);
        SharedMemory shm("/cryptstream_queue", shm_size, true);
        
        // Create task queue
        TaskQueue queue(shm, true);
        
        // Create semaphores
        Semaphore task_sem("/cryptstream_task_sem", 0, true);
        Semaphore done_sem("/cryptstream_done_sem", 0, true);
        
        // Create and start process pool
        ProcessPool pool(config.num_processes, queue, task_sem, done_sem);
        pool.start();
        
        // Enqueue task
        Task task;
        task.type = (config.command == "encrypt") ? Task::ENCRYPT : Task::DECRYPT;
        task.set_input(config.input_file);
        task.set_output(config.output_file);
        task.set_key(config.key);
        
        if (!queue.enqueue(task)) {
            std::cerr << "Failed to enqueue task" << std::endl;
            return 1;
        }
        
        // Signal task availability
        task_sem.post();
        
        // Wait for task completion
        done_sem.wait();
        
        // Signal shutdown
        queue.signal_shutdown();
        for (size_t i = 0; i < config.num_processes; ++i) {
            task_sem.post();
        }
        
        // Wait for all workers
        pool.wait_all();
        
        // Cleanup
        shm.unlink();
        task_sem.unlink();
        done_sem.unlink();
        
        std::cout << "File processed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
