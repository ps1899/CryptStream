CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -pthread
LDFLAGS = -pthread

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
COMMON_SOURCES = $(filter-out $(SRC_DIR)/main.cpp $(SRC_DIR)/benchmark.cpp, $(wildcard $(SRC_DIR)/*.cpp))
COMMON_OBJECTS = $(COMMON_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Targets
TARGET = $(BIN_DIR)/cryptstream
BENCHMARK = $(BIN_DIR)/benchmark

.PHONY: all clean directories

all: directories $(TARGET) $(BENCHMARK)

directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(TARGET): $(COMMON_OBJECTS) $(BUILD_DIR)/main.o
	$(CXX) $(LDFLAGS) -o $@ $^

$(BENCHMARK): $(COMMON_OBJECTS) $(BUILD_DIR)/benchmark.o
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@echo "Running tests..."
	@./tests/run_tests.sh

benchmark: $(BENCHMARK)
	./$(BENCHMARK)

.PHONY: help
help:
	@echo "CryptStream Build System"
	@echo "========================"
	@echo "make all       - Build everything"
	@echo "make clean     - Remove build artifacts"
	@echo "make run       - Build and run"
	@echo "make test      - Run tests"
	@echo "make benchmark - Run benchmarks"
