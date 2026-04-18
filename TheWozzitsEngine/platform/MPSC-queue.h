#pragma once

#include <atomic>
#include <utility>

namespace WZ {

template <typename T>
class MPSCQueue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:
    MPSCQueue() {
        Node* dummy = new Node(T{});
        head.store(dummy, std::memory_order_relaxed);
        tail.store(dummy, std::memory_order_relaxed);
    }

    ~MPSCQueue() {
        clear();
        delete head.load(std::memory_order_relaxed);
    }

    // Prevent copying/moving
    MPSCQueue(const MPSCQueue&) = delete;
    MPSCQueue& operator=(const MPSCQueue&) = delete;
    MPSCQueue(MPSCQueue&&) = delete;
    MPSCQueue& operator=(MPSCQueue&&) = delete;

    void push(T value) {
        Node* node = new Node(std::move(value));
        Node* prev = tail.load(std::memory_order_relaxed);
        while (true) {
            Node* next = prev->next.load(std::memory_order_relaxed);
            if (prev == tail.load(std::memory_order_relaxed)) {
                if (!next) {
                    if (prev->next.compare_exchange_weak(next, node, std::memory_order_release, std::memory_order_relaxed)) {
                        tail.compare_exchange_strong(prev, node, std::memory_order_release, std::memory_order_relaxed);
                        return;
                    }
                } else {
                    prev = next;
                }
            } else {
                prev = next;
            }
        }
    }

    bool try_pop(T& out) {
        Node* h = head.load(std::memory_order_relaxed);
        Node* t = tail.load(std::memory_order_relaxed);
        Node* next = h->next.load(std::memory_order_acquire);
        if (h == t) {
            return false;
        }
        if (next) {
            out = std::move(next->data);
            head.store(next, std::memory_order_release);
            delete h;
            return true;
        }
        return false;
    }

    bool empty() const {
        return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
    }

    void clear() {
        T dummy{};
        while (try_pop(dummy)) {}
    }
};

} // namespace WZ
