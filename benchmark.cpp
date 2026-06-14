#include "lru_cache.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>

int main() {
    constexpr std::size_t capacity = 10000;
    constexpr std::size_t operations = 1000000;

    cache::LRUCache<int, std::string> cache(capacity);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> key_dist(1, 50000);
    std::bernoulli_distribution write_dist(0.35);

    const auto start = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < operations; ++i) {
        const int key = key_dist(rng);
        if (write_dist(rng)) {
            cache.put(key, "value-" + std::to_string(key));
        } else {
            (void)cache.get(key);
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    const auto& stats = cache.stats();
    std::cout << "operations: " << operations << '\n';
    std::cout << "capacity: " << capacity << '\n';
    std::cout << "final size: " << cache.size() << '\n';
    std::cout << "elapsed: " << elapsed_us << " us\n";
    std::cout << "average: " << static_cast<double>(elapsed_us) / operations
              << " us/op\n";
    std::cout << "hits: " << stats.hits << '\n';
    std::cout << "misses: " << stats.misses << '\n';
    std::cout << "evictions: " << stats.evictions << '\n';
    std::cout << "hit rate: " << stats.hit_rate() * 100.0 << "%\n";

    return 0;
}
