#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cache {

struct CacheStats {
    std::size_t hits = 0;
    std::size_t misses = 0;
    std::size_t evictions = 0;

    [[nodiscard]] double hit_rate() const noexcept {
        const auto total = hits + misses;
        return total == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(total);
    }
};

enum class PutResult {
    Inserted,
    Updated,
    Evicted
};

template <typename Key,
          typename Value,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class LRUCache {
private:
    struct Node {
        explicit Node(Value value, const Key* key)
            : value(std::move(value)), key(key) {}

        Value value;
        const Key* key;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    using Map = std::unordered_map<Key, std::unique_ptr<Node>, Hash, KeyEqual>;

public:
    explicit LRUCache(std::size_t capacity)
        : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("LRUCache capacity must be greater than zero");
        }

        index_.reserve(capacity_);
    }

    LRUCache(std::size_t capacity, std::initializer_list<std::pair<Key, Value>> values)
        : LRUCache(capacity) {
        for (const auto& entry : values) {
            put(entry.first, entry.second);
        }
    }

    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    LRUCache(LRUCache&&) = delete;
    LRUCache& operator=(LRUCache&&) = delete;

    [[nodiscard]] std::size_t size() const noexcept {
        return index_.size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return capacity_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return index_.empty();
    }

    [[nodiscard]] bool contains(const Key& key) const {
        return index_.find(key) != index_.end();
    }

    [[nodiscard]] const CacheStats& stats() const noexcept {
        return stats_;
    }

    void reset_stats() noexcept {
        stats_ = {};
    }

    std::optional<Value> get(const Key& key) {
        auto* node = find_node(key);
        if (node == nullptr) {
            ++stats_.misses;
            return std::nullopt;
        }

        ++stats_.hits;
        move_to_front(node);
        return node->value;
    }

    bool get(const Key& key, Value& out) {
        auto value = get(key);
        if (!value.has_value()) {
            return false;
        }

        out = std::move(*value);
        return true;
    }

    PutResult put(const Key& key, Value value) {
        if (auto* existing = find_node(key)) {
            existing->value = std::move(value);
            move_to_front(existing);
            return PutResult::Updated;
        }

        const bool will_evict = index_.size() == capacity_;
        if (will_evict) {
            evict_lru();
        }

        insert_new(key, std::move(value));
        return will_evict ? PutResult::Evicted : PutResult::Inserted;
    }

    bool erase(const Key& key) {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return false;
        }

        unlink(it->second.get());
        index_.erase(it);
        return true;
    }

    void clear() noexcept {
        index_.clear();
        head_ = nullptr;
        tail_ = nullptr;
    }

    [[nodiscard]] std::vector<Key> keys_mru_to_lru() const {
        std::vector<Key> keys;
        keys.reserve(index_.size());

        for (Node* current = head_; current != nullptr; current = current->next) {
            keys.push_back(*current->key);
        }

        return keys;
    }

private:
    Node* find_node(const Key& key) {
        auto it = index_.find(key);
        return it == index_.end() ? nullptr : it->second.get();
    }

    void insert_new(const Key& key, Value value) {
        auto [it, inserted] = index_.emplace(key, nullptr);
        if (!inserted) {
            throw std::logic_error("attempted to insert duplicate cache key");
        }

        try {
            it->second = std::make_unique<Node>(std::move(value), &it->first);
            push_front(it->second.get());
        } catch (...) {
            index_.erase(it);
            throw;
        }
    }

    void evict_lru() {
        Node* victim = tail_;
        if (victim == nullptr) {
            return;
        }

        Key key = *victim->key;
        unlink(victim);
        index_.erase(key);
        ++stats_.evictions;
    }

    void move_to_front(Node* node) noexcept {
        if (node == head_) {
            return;
        }

        unlink(node);
        push_front(node);
    }

    void push_front(Node* node) noexcept {
        node->prev = nullptr;
        node->next = head_;

        if (head_ != nullptr) {
            head_->prev = node;
        }

        head_ = node;
        if (tail_ == nullptr) {
            tail_ = node;
        }
    }

    void unlink(Node* node) noexcept {
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head_ = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail_ = node->prev;
        }

        node->prev = nullptr;
        node->next = nullptr;
    }

    std::size_t capacity_;
    Map index_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    CacheStats stats_;
};

}  // namespace cache
