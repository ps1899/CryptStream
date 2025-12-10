#ifndef CRYPTSTREAM_FILE_PROCESSOR_HPP
#define CRYPTSTREAM_FILE_PROCESSOR_HPP

#include "crypto.hpp"
#include "task_queue.hpp"
#include <fstream>
#include <vector>
#include <memory>
#include <string>

namespace cryptstream {

/**
 * File processor using std::fstream with std::move for ownership transfer
 * Handles file I/O for encryption/decryption operations
 */
class FileProcessor {
public:
    FileProcessor() = default;
    
    // Process a single file (encrypt or decrypt)
    static bool process_file(const Task& task);
    
    // Read file into buffer
    static std::vector<uint8_t> read_file(std::ifstream&& input);
    
    // Write buffer to file
    static void write_file(std::ofstream&& output, const std::vector<uint8_t>& data);
    
    // Get file size
    static size_t get_file_size(const std::string& filepath);
    
private:
    static constexpr size_t BUFFER_SIZE = 8192;  // 8KB buffer
};

} // namespace cryptstream

#endif // CRYPTSTREAM_FILE_PROCESSOR_HPP
