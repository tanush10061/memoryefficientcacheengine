CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude
DEBUG_FLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -Iinclude
BUILD_DIR := build

.PHONY: all demo test bench debug clean

all: demo test bench

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

demo: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(BUILD_DIR)/lru_demo

test: $(BUILD_DIR)
	$(CXX) $(DEBUG_FLAGS) tests/lru_cache_tests.cpp -o $(BUILD_DIR)/lru_cache_tests
	./$(BUILD_DIR)/lru_cache_tests

bench: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) benchmarks/benchmark.cpp -o $(BUILD_DIR)/benchmark
	./$(BUILD_DIR)/benchmark

debug: $(BUILD_DIR)
	$(CXX) $(DEBUG_FLAGS) src/main.cpp -o $(BUILD_DIR)/lru_demo_debug

clean:
	rm -rf $(BUILD_DIR)
