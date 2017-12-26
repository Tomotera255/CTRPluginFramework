extern "C"
{
    #include "types.h"
    #include "ctrulib/util/rbtree.h"
}

#include "../../ctrulib/allocator/mem_pool.h"
#include "../../ctrulib/allocator/addrmap.h"
#include "CTRPluginFrameworkImpl/System/Heap.hpp"

void abort(void);

namespace std
{
    void memcpy(void *dst, void *src, size_t size);
}
namespace CTRPluginFramework
{
    namespace Heap
    {
        u32     __ctrpf_heap = 0;
        u32     __ctrpf_heap_size = 0;

        static MemPool  g_heapPool;

        static bool     HeapInit(void)
        {
            if (!__ctrpf_heap || !__ctrpf_heap_size)
                abort();

            MemBlock    *blk = MemBlock::Create(reinterpret_cast<u8 *>(__ctrpf_heap), __ctrpf_heap_size);

            if (blk)
            {
                g_heapPool.AddBlock(blk);
                rbtree_init(&sAddrMap, addrMapNodeComparator);
                return true;
            }
            return false;
        }

        void    *MemAlign(size_t size, size_t alignment)
        {
            // Enforce minimum alignment
            if (alignment < 16)
                alignment = 16;

            // Convert alignment to shift amount
            int shift;
            for (shift = 4; shift < 32; shift++)
            {
                if ((1U << shift) == alignment)
                    break;
            }
            if (shift == 32) // Invalid alignment
                return nullptr;

            // Initialize the pool if it is not ready
            if (!g_heapPool.Ready() && !HeapInit())
                return nullptr;

            // Allocate the chunk
            MemChunk chunk;
            if (!g_heapPool.Allocate(chunk, size, shift))
                return nullptr;

            auto node = newNode(chunk);
            if (!node)
            {
                g_heapPool.Deallocate(chunk);
                return nullptr;
            }
            rbtree_node_t *t = rbtree_insert(&sAddrMap, &node->node);
            return chunk.addr;
        }

        void    *Alloc(const size_t size)
        {
            return (MemAlign(size, 16));
        }

        void    *Realloc(void *ptr, const size_t size)
        {
            addrMapNode   *node = getNode(ptr);
            addrMapNode   *newnode;

            if (!node) return nullptr;

            MemChunk    &chunk = node->chunk;
            MemChunk    newChunk;

            // If new size is lower, we don't care
            if (size < chunk.size)
                return chunk.addr;

            // Create a new chunk
            if (!g_heapPool.Allocate(newChunk, size, 4))
            {
                // Chunk creation failed, return nullptr and do nothing as realloc man says
                return nullptr;
            }

            // Create a new node
            newnode = newNode(newChunk);
            if (!newnode)
            {
                g_heapPool.Deallocate(chunk);
                return nullptr;
            }
            if (rbtree_insert(&sAddrMap, &newnode->node));

            // Copy the old chunk data to the new one
            std::memcpy(newChunk.addr, chunk.addr, chunk.size);

            // Free old chunk
            g_heapPool.Deallocate(chunk);

            // Free old node
            delNode(node);

            // Return the new address
            return newChunk.addr;
        }

        void    Free(void *ptr)
        {
            addrMapNode   *node = getNode(ptr);

            if (!node) return;

            // Free the chunk
            g_heapPool.Deallocate(node->chunk);

            // Free the node
            delNode(node);
        }

        size_t  SpaceFree(void)
        {
            return g_heapPool.GetFreeSpace();
        }
    }
}