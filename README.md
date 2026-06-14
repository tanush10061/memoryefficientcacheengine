# Memory-Efficient LRU Cache Engine

A small C++17 implementation of an LRU cache using a hash map plus an intrusive doubly linked list. The goal is to make `get` and `put` run in average `O(1)` time while keeping the memory layout explicit enough to explain clearly in interviews.


## Architecture

The cache combines two data structures:

1. `std::unordered_map<Key, std::unique_ptr<Node>>`
   - Finds a key in average `O(1)` time.
   - Owns each node with RAII.

2. Intrusive doubly linked list
   - Every node has `prev` and `next` pointers.
   - `head` is the most recently used item.
   - `tail` is the least recently used item.
   - Moving a node to the front is `O(1)`.
   - Evicting the tail is `O(1)`.

```text
Hash Map                         Doubly Linked List
--------                         ------------------
key 1 -> Node* --------------->  [MRU] <-> [ ... ] <-> [LRU]
key 2 -> Node*
key 3 -> Node*
```

## Memory Layout

Each cache entry is stored once as a heap-allocated node owned by the map:

```cpp
struct Node {
    Value value;
    const Key* key;
    Node* prev;
    Node* next;
};
```

The node stores a pointer to the key already owned by the `unordered_map`, which avoids duplicating the key inside the node. The `prev` and `next` pointers are non-owning links used only to maintain recency order.

## Complexity

| Operation | Average Time | Notes |
| --- | ---: | --- |
| `get(key)` | `O(1)` | Hash lookup, then move node to front |
| `put(key, value)` | `O(1)` | Insert/update, possibly evict tail |
| `erase(key)` | `O(1)` | Hash lookup, unlink node |
| `keys_mru_to_lru()` | `O(n)` | Debug/visualization helper |

## Build and Run

This project uses a simple `Makefile`, so CMake is not required.

```bash
make demo
./build/lru_demo
```

Run tests:

```bash
make test
```

Run benchmark:

```bash
make bench
```

Build everything:

```bash
make all
```

## Example Usage

```cpp
#include "lru_cache.hpp"

cache::LRUCache<int, std::string> cache(2);

cache.put(1, "one");
cache.put(2, "two");
cache.get(1);          // key 1 becomes most recently used
cache.put(3, "three"); // evicts key 2
```

## Interview Explanation

An LRU cache must answer two questions quickly:

1. Does this key exist?
2. Which item should be evicted when the cache is full?

A hash map answers the first question in average `O(1)`. A doubly linked list answers the second because the least recently used item is always at the tail. Whenever a key is accessed or updated, its node is moved to the head. When capacity is full, the tail node is removed and its key is erased from the map.

The important invariant is:

```text
head = most recently used
tail = least recently used
map[key] = exact node address in the linked list
```

As long as every `get`, `put`, and `erase` preserves that invariant, the cache remains correct.

## Possible Extensions

- Add TTL expiration per key.
- Add thread safety with `std::mutex` or `std::shared_mutex`.
- Add a memory budget in bytes instead of item count.
- Add metrics export for hit rate and eviction count.
- Add an LFU or segmented LRU policy for comparison.
