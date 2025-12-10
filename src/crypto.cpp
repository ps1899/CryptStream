#include "crypto.hpp"
#include <algorithm>
#include <stdexcept>

namespace cryptstream {

Crypto::Crypto(const std::string& key) : key_index_(0) {
    if (key.empty()) {
        throw std::invalid_argument("Encryption key cannot be empty");
    }
    expandKey(key);
}

void Crypto::expandKey(const std::string& key) {
    // Simple key expansion: repeat key to fill 256 bytes
    key_.reserve(256);
    for (size_t i = 0; i < 256; ++i) {
        key_.push_back(static_cast<uint8_t>(key[i % key.size()]));
    }
}

void Crypto::encrypt(std::vector<uint8_t>& data) {
    process(data);
}

void Crypto::decrypt(std::vector<uint8_t>& data) {
    process(data);
}

void Crypto::process(std::vector<uint8_t>& data) {
    // XOR encryption/decryption
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key_[(key_index_ + i) % key_.size()];
    }
    key_index_ = (key_index_ + data.size()) % key_.size();
}

} // namespace cryptstream
