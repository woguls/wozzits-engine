#pragma once
#include <vector>
#include <atomic>
#include <utility>
#include <mutex>
namespace wz::core::internal
{
    template <typename T>
    class MPSCQueue
    {
    private:
        struct Node
        {
            alignas(64) std::atomic<Node *> next;
            T data;

            Node() : next(nullptr), data() {} // dummy node
            Node(T value) : next(nullptr), data(std::move(value)) {}
        };

        alignas(64) std::atomic<Node *> head;
        alignas(64) std::atomic<Node *> tail;

        std::atomic<Node *> free_list{nullptr};

        static constexpr size_t BLOCK_SIZE = 256;

        static constexpr size_t CAPACITY = 64 * 1024; // tune this
        alignas(64) Node *arena;

        void free_node(Node *node)
        {
            Node *head = free_list.load(std::memory_order_relaxed);

            do
            {
                node->next.store(head, std::memory_order_relaxed);
            } while (!free_list.compare_exchange_weak(
                head,
                node,
                std::memory_order_release,
                std::memory_order_relaxed));
        }

        Node *allocate_node()
        {
            for (;;)
            {
                Node *node = free_list.load(std::memory_order_acquire);

                if (!node)
                    return nullptr;

                Node *next = node->next.load(std::memory_order_relaxed);

                if (free_list.compare_exchange_weak(
                        node,
                        next,
                        std::memory_order_acquire,
                        std::memory_order_relaxed))
                {
                    node->next.store(nullptr, std::memory_order_relaxed); // IMPORTANT
                    return node;
                }
            }
        }

    public:
        MPSCQueue()
        {
            arena = static_cast<Node *>(
                operator new[](sizeof(Node) * CAPACITY));

            // dummy is reserved
            Node *dummy = new (&arena[0]) Node();

            // freelist starts at arena[1]
            Node *prev = nullptr;

            for (size_t i = 1; i < CAPACITY; ++i)
            {
                Node *node = new (&arena[i]) Node();

                node->next.store(prev, std::memory_order_relaxed);
                prev = node;
            }

            free_list.store(prev, std::memory_order_relaxed);

            head.store(dummy, std::memory_order_relaxed);
            tail.store(dummy, std::memory_order_relaxed);
        }

        ~MPSCQueue()
        {
            T tmp;
            while (try_pop(tmp))
            {
            }

            free_list.store(nullptr, std::memory_order_relaxed);

            // for (Node *block : blocks)
            // {
            //     operator delete[](block);
            // }
        }

        MPSCQueue(const MPSCQueue &) = delete;
        MPSCQueue &operator=(const MPSCQueue &) = delete;

        void push(T value)
        {

            Node *node;

            for (;;)
            {
                node = allocate_node();
                if (node)
                    break;

                std::this_thread::yield(); // or spin-wait
            }
            node->data = std::move(value);
            node->next.store(nullptr, std::memory_order_relaxed);

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

            free_node(head_node);
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