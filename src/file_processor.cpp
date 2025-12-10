#include "file_processor.hpp"
#include <iostream>
#include <sys/stat.h>
namespace cryptstream {

bool FileProcessor::process_file(const Task& task) {
    try {
        // Open input file with std::move for ownership transfer
        std::ifstream input(task.input_file, std::ios::binary);
        if (!input.is_open()) {
            std::cerr << "Failed to open input file: " << task.input_file << std::endl;
            return false;
        }
        
        // Read file data using std::move
        std::vector<uint8_t> data = read_file(std::move(input));
        
        // Create crypto instance
        Crypto crypto(task.key);
        
        // Process data based on task type
        if (task.type == Task::ENCRYPT) {
            crypto.encrypt(data);
        } else if (task.type == Task::DECRYPT) {
            crypto.decrypt(data);
        }
        
        // Open output file with std::move for ownership transfer
        std::ofstream output(task.output_file, std::ios::binary);
        if (!output.is_open()) {
            std::cerr << "Failed to open output file: " << task.output_file << std::endl;
            return false;
        }
        
        // Write processed data using std::move
        write_file(std::move(output), data);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error processing file: " << e.what() << std::endl;
        return false;
    }
}

std::vector<uint8_t> FileProcessor::read_file(std::ifstream&& input) {
    // Move ownership of the stream
    std::ifstream file = std::move(input);
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read entire file into buffer
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    // Stream automatically closed when it goes out of scope
    return buffer;
}

void FileProcessor::write_file(std::ofstream&& output, const std::vector<uint8_t>& data) {
    // Move ownership of the stream
    std::ofstream file = std::move(output);
    
    // Write data to file
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    
    // Stream automatically closed when it goes out of scope
}

size_t FileProcessor::get_file_size(const std::string& filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) == 0) {
        return st.st_size;
    }
    return 0;
}

} // namespace cryptstream
