# CryptStream

CryptStream is a high-performance, CLI-based file encryption and decryption system built in C++17. It uses AES-GCM encryption for authenticated security, a thread pool architecture for parallel processing, and is containerized with Docker and CI/CD ready via GitHub Actions.

---

## 🔐 Features

- 🔄 AES-GCM 128-bit authenticated encryption using OpenSSL
- ⚙️ Thread pool-based task execution for parallelism and scalability
- 🧪 Modular design for unit testing (task queue, encryption, threading)
- 🐳 Dockerfile for consistent, portable builds
- ✅ GitHub Actions CI pipeline for automated testing
- 📈 Benchmark-friendly architecture

---

## 🚀 Getting Started

### 🔧 Build Instructions

**Dependencies:**
- g++ (C++17)
- OpenSSL (`libssl-dev`)
- Make

```bash
make
```

---

## 🧪 Usage

```bash
# Encrypt
./encrypt_engine encrypt input.txt encrypted.bin

# Decrypt
./encrypt_engine decrypt encrypted.bin output.txt

# Verify
diff input.txt output.txt && echo "✅ Files match"
```

---

## 🧪 Unit Testing

This project is designed for GoogleTest integration.

Testable components include:
- AES encryption/decryption engine (`aes_gcm_crypt_file`)
- Task queue behavior (`worker_thread`, `execute_task`)
- Threaded task dispatch logic

---

## 🐳 Docker Support

A `Dockerfile` is provided.

```bash
docker build -t securefileengine .
docker run -v $PWD:/app securefileengine encrypt input.txt encrypted.bin
```

---

## ✅ CI/CD Integration

GitHub Actions workflow runs on every push/pull request and includes:
- Build from source
- Encrypt and decrypt roundtrip test
- Output verification

---

## 📊 Benchmark

Multithreaded AES-GCM encryption shows **250%+ performance gain** on files > 400KB compared to single-threaded implementation.

---

## 📁 Project Structure

- `encrypt_engine.cpp` – Main logic for encryption and thread execution
- `Makefile` – Build script
- `Dockerfile` – Containerized build environment
- `.github/workflows/ci.yml` – CI pipeline definition

---

## 📌 License

This project is released under the MIT License.
