#include "lru_cache.hpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

template <typename T>
void assert_vector_eq(const std::vector<T>& actual, const std::vector<T>& expected) {
    assert(actual == expected);
}

void rejects_zero_capacity() {
    bool threw = false;
    try {
        cache::LRUCache<int, int> bad_cache(0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void inserts_and_reads_values() {
    cache::LRUCache<int, std::string> cache(2);

    assert(cache.put(1, "one") == cache::PutResult::Inserted);
    assert(cache.put(2, "two") == cache::PutResult::Inserted);

    assert(cache.get(1).value() == "one");
    assert(cache.get(2).value() == "two");
    assert(cache.stats().hits == 2);
    assert(cache.stats().misses == 0);
}

void get_moves_entry_to_front() {
    cache::LRUCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);

    assert_vector_eq(cache.keys_mru_to_lru(), std::vector<int>({3, 2, 1}));

    assert(cache.get(1).value() == 10);

    assert_vector_eq(cache.keys_mru_to_lru(), std::vector<int>({1, 3, 2}));
}

void evicts_least_recently_used_entry() {
    cache::LRUCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);

    assert(cache.get(1).value() == 10);
    assert(cache.put(3, 30) == cache::PutResult::Evicted);

    assert(cache.get(2) == std::nullopt);
    assert(cache.get(1).value() == 10);
    assert(cache.get(3).value() == 30);
    assert(cache.stats().evictions == 1);
}

void updates_existing_entry_without_growing() {
    cache::LRUCache<int, std::string> cache(2);
    cache.put(1, "old");
    cache.put(2, "two");

    assert(cache.put(1, "new") == cache::PutResult::Updated);
    assert(cache.size() == 2);
    assert(cache.get(1).value() == "new");
    assert_vector_eq(cache.keys_mru_to_lru(), std::vector<int>({1, 2}));
}

void erase_and_clear_keep_list_consistent() {
    cache::LRUCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);

    assert(cache.erase(2));
    assert(!cache.contains(2));
    assert_vector_eq(cache.keys_mru_to_lru(), std::vector<int>({3, 1}));

    assert(cache.erase(3));
    assert_vector_eq(cache.keys_mru_to_lru(), std::vector<int>({1}));

    cache.clear();
    assert(cache.empty());
    assert(cache.keys_mru_to_lru().empty());
}

}  // namespace

int main() {
    rejects_zero_capacity();
    inserts_and_reads_values();
    get_moves_entry_to_front();
    evicts_least_recently_used_entry();
    updates_existing_entry_without_growing();
    erase_and_clear_keep_list_consistent();

    return 0;
}
