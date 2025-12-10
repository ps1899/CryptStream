#ifndef CRYPTSTREAM_CRYPTO_HPP
#define CRYPTSTREAM_CRYPTO_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace cryptstream {

/**
 * Simple XOR-based encryption for demonstration
 * In production, use AES or other standard algorithms
 */
class Crypto {
public:
    explicit Crypto(const std::string& key);
    
    // Encrypt data in-place
    void encrypt(std::vector<uint8_t>& data);
    
    // Decrypt data in-place
    void decrypt(std::vector<uint8_t>& data);
    
    // Encrypt/decrypt are symmetric for XOR
    void process(std::vector<uint8_t>& data);
    
private:
    std::vector<uint8_t> key_;
    size_t key_index_;
    
    void expandKey(const std::string& key);
};

} // namespace cryptstream

#endif // CRYPTSTREAM_CRYPTO_HPP
