#pragma once

#include <atomic>
#include <utility>

namespace wz::core::internal
{
    template <typename T>
    class MPSCQueue
    {
    private:
        struct Node
        {
            std::atomic<Node *> next;
            T data;

            Node() : next(nullptr), data() {} // dummy node
            Node(T value) : next(nullptr), data(std::move(value)) {}
        };

        std::atomic<Node *> head;
        std::atomic<Node *> tail;

    public:
        MPSCQueue()
        {
            Node *dummy = new Node();
            head.store(dummy, std::memory_order_relaxed);
            tail.store(dummy, std::memory_order_relaxed);
        }

        ~MPSCQueue()
        {
            T tmp;
            while (try_pop(tmp))
            {
            }

            Node *node = head.load(std::memory_order_relaxed);
            delete node;
        }

        MPSCQueue(const MPSCQueue &) = delete;
        MPSCQueue &operator=(const MPSCQueue &) = delete;

        void push(T value)
        {
            Node *node = new Node(std::move(value));

            Node *prev = tail.exchange(node, std::memory_order_acq_rel);
            prev->next.store(node, std::memory_order_release);
        }

        bool try_pop(T &out)
        {
            Node *head_node = head.load(std::memory_order_acquire);
            Node *next = head_node->next.load(std::memory_order_acquire);

            if (!next)
                return false;

            out = std::move(next->data);
            head.store(next, std::memory_order_release);
            delete head_node;

            return true;
        }

        bool empty() const
        {
            Node *head_node = head.load(std::memory_order_acquire);
            Node *next = head_node->next.load(std::memory_order_acquire);
            return next == nullptr;
        }

        void clear()
        {
            T tmp;
            while (try_pop(tmp))
            {
            }
        }
    };

} // namespace WZ