#include "lru_cache.hpp"

#include <iomanip>
#include <iostream>
#include <string>

namespace {

void print_cache(const cache::LRUCache<int, std::string>& cache) {
    std::cout << "MRU -> LRU: ";
    const auto keys = cache.keys_mru_to_lru();

    if (keys.empty()) {
        std::cout << "(empty)";
    }

    for (const auto key : keys) {
        std::cout << key << ' ';
    }

    std::cout << '\n';
}

void print_get(cache::LRUCache<int, std::string>& cache, int key) {
    const auto value = cache.get(key);
    if (value.has_value()) {
        std::cout << "get(" << key << ") = " << *value << '\n';
    } else {
        std::cout << "get(" << key << ") = MISS\n";
    }

    print_cache(cache);
}

}  // namespace

int main() {
    cache::LRUCache<int, std::string> pages(3);

    std::cout << "Memory-Efficient LRU Cache Demo\n\n";

    pages.put(1, "profile:tanush");
    pages.put(2, "feed:page:1");
    pages.put(3, "settings");
    print_cache(pages);

    print_get(pages, 1);

    std::cout << "put(4, notifications) evicts key 2 because it is least recently used\n";
    pages.put(4, "notifications");
    print_cache(pages);

    print_get(pages, 2);
    print_get(pages, 3);

    const auto& stats = pages.stats();
    std::cout << "\nStats\n";
    std::cout << "hits: " << stats.hits << '\n';
    std::cout << "misses: " << stats.misses << '\n';
    std::cout << "evictions: " << stats.evictions << '\n';
    std::cout << "hit rate: " << std::fixed << std::setprecision(2)
              << stats.hit_rate() * 100.0 << "%\n";

    return 0;
}
