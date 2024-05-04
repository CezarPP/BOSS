/*
 * buddy_allocator.h
 *
 *  Created on: 4/30/24.
 *      Author: Cezar PP
 */


#pragma once

/*
 * basic_allocator.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */

#pragma once

#include "allocator.h"
#include "std/bitset.h"
#include "std/vector.h"
#include "std/array.h"

namespace physical_allocator {
    /*!
     * A buddy allocator
     *
     * The Buddy Allocator will keep a binary tree.
     * For each level there will be a free list
     *
     * Layer - layer 0 is root, leafs are last layer, maxOrder_ - number of layers
     * Order - the power of 2 you have the multiply with the smallest block size
     * order 0 - smallest block size, bottom layer
     * blockSize for a layer = PAGE_SIZE * 2^order = PAGE_SIZE * 2^(n - 1 - layer)
     *
     */
    class BuddyAllocator : public Allocator<BuddyAllocator> {
    private:
        constexpr static size_t FREE_LIST_NODE_SIZE = sizeof(size_t);
        /// maxOrder_ has to be less than or equal to MAX_LEVEL
        static constexpr const size_t MAX_LEVEL = 23;
        /// The order of the top node, order for bottom layer is 0
        const size_t maxOrder_;
        /// The number of pages the allocator will keep track of, this is (1 << maxOrder_)
        const size_t cntPages_;
        /// The number of nodes the tree will have
        const size_t cntNodes_;

        /// The number of bytes that the allocator for tree_ will get
        const size_t treeSize_;
        /**
         * The tree that will keep the buddies, represented as an array
         * A node in the tree is set to true if either it is busy or one of its children is busy
         */
        std::vector<bool, physicalStdAllocator<bool>> tree_;

        /// The number of bytes that the allocator for freeLists_ will get
        const size_t freeListsSize_;
        /// A free list for each level of the tree, from 0 to maxOrder_
        std::vector<std::vector<size_t, physicalStdAllocator<size_t>>,
                physicalStdAllocator<std::vector<size_t, physicalStdAllocator<size_t>>>> freeLists_;

        [[nodiscard]] constexpr size_t orderToLayer(size_t order) const {
            return maxOrder_ - order;
        }

        [[nodiscard]] static constexpr size_t layerOffset(size_t layer) {
            return (1ULL << layer) - 1;
        }

        [[nodiscard]] static constexpr size_t largestPowerOf2LE(size_t x) {
            for (int i = 63; i >= 0; i--)
                if (x & (1ULL << i))
                    return i;
            kPanic("Memory size cannot be zero");
            return 0;
        }

        [[nodiscard]] static constexpr size_t smallestPowerOf2GE(size_t x) {
            kAssert(x != 0, "x should not be zero here");

            if ((x & (x - 1)) == 0) {
                // x is a power of 2
                return x;
            }

            // x is not a power of 2, get the next largest power
            for (int i = 63; i >= 0; i--)
                if (x & (1ULL << i))
                    return i + 1;
            kPanic("We can't get here");
            return 0;
        }

        /*!
        * Returns the buddy (sibling) index of the specified node in a buddy system tree
        * @param nodeIndex Index of the node whose buddy is to be found
        * @return Index of the buddy node
        */
        constexpr static size_t getBuddyIndex(size_t nodeIndex) {
            // Check if the node is left (odd index) or right (even index) child
            if (nodeIndex % 2 == 0) {
                // node is right child, buddy is left child
                return nodeIndex - 1;
            } else {
                // node is left child, buddy is right child
                return nodeIndex + 1;
            }
        }

    public:
        BuddyAllocator(size_t memBase, size_t memSize)
                : Allocator(memBase, memSize),
                  maxOrder_{largestPowerOf2LE(memSize / PAGE_SIZE)},
                  cntPages_{(1ULL << maxOrder_)},
                  cntNodes_{(1ULL << (maxOrder_ + 1)) - 1},
                  treeSize_{cntNodes_},
                  tree_{physicalStdAllocator(static_cast<bool *>(allocatorMemory) + PAGE_SIZE, treeSize_)},
                  freeListsSize_{cntNodes_ * FREE_LIST_NODE_SIZE},
                  freeLists_{physicalStdAllocator<std::vector<size_t, physicalStdAllocator<size_t>>>(
                          (std::vector<size_t, physicalStdAllocator<size_t>> *) allocatorMemory,
                          freeListsSize_)} {
            Logger::instance().println("[P_ALLOCATOR] Initializing buddy allocator...");
            Logger::instance().println("[P_ALLOCATOR] maxOrder_ = %X, treeSize_ = %X", maxOrder_, treeSize_);
            kAssert(paging::pageAligned(this->memBase), "memBase is not page aligned");
            kAssert(maxOrder_ <= MAX_LEVEL, "maxOrder cannot be bigger than MAX_LEVEL");

            tree_.resize(treeSize_);
            std::fill(tree_.begin(), tree_.end(), false);

            Logger::instance().println("[P_ALLOCATOR] Initializing the free lists...");
            void *freeListMemory = static_cast<uint8_t *>(allocatorMemory) + PAGE_SIZE + treeSize_;
            // Initialize the freeLists_
            size_t pagesPerLevel = cntPages_;
            size_t currentOffset = 0;
            kAssert(freeLists_.empty(), "freeLists_ should be empty");
            freeLists_.reserve(maxOrder_ + 1);
            for (size_t i = 0; i <= maxOrder_; i++) {
                size_t levelMemSize = pagesPerLevel * sizeof(size_t); // Calculate memory needed for this level
                Logger::instance().println(
                        "[P_ALLOCATOR] Initializing the free lists %d, levelMemSize = %X, pagesPerLevel = %X",
                        i, levelMemSize, pagesPerLevel);

                freeLists_.push_back(std::move(std::vector<size_t, physicalStdAllocator<size_t>>(
                        physicalStdAllocator<size_t>(
                                reinterpret_cast<size_t *>(static_cast<uint8_t *>(freeListMemory) + currentOffset),
                                levelMemSize))));
                freeLists_[i].reserve(pagesPerLevel);

                currentOffset += levelMemSize; // Adjust offset for next level
                pagesPerLevel /= 2; // Adjust the number of pages per level for the next iteration
            }

            /// Add the root to the free list, the root node has index 0
            freeLists_[maxOrder_].push_back(0);

            Logger::instance().println("[P_ALLOCATOR] Buddy allocator initialized successfully!");
        }


        [[nodiscard]] constexpr size_t nodeToPhysicalAddress(size_t node, size_t order) const {
            return this->memBase + (node - layerOffset(orderToLayer(order))) * PAGE_SIZE * (1ULL << order);
        }

        /*!
         * Allocated cntPages pages, consecutive in physical memory
         * @param cntPages The number of cntPages to allocate
         * @return The physical address of the first page
         */
        size_t allocateImplementation(size_t cntPages) {
            size_t order = smallestPowerOf2GE(cntPages);

            /// Check free lists for that order
            if (!freeLists_[order].empty()) {
                /// Return element from free list
                size_t node = freeLists_[order].back();
                freeLists_[order].pop_back();
                tree_[node] = true;
                Logger::instance().println("Returning node %X", node);
                return nodeToPhysicalAddress(node, order);
            } else {
                /// Break some higher order pair
                auto higherOrder = order + 1;

                /// Find the smaller order that has a free node
                while (freeLists_[higherOrder].empty() && higherOrder <= maxOrder_) {
                    higherOrder++;
                }
                kAssert(!freeLists_[higherOrder].empty(), "No free page found");

                /// Remove the node from its free list
                size_t node = freeLists_[higherOrder].back();
                freeLists_[higherOrder].pop_back();
                kAssert(!tree_[node], "Node from free list can't be occupied");

                /// Move down from the node to find a block with a suitable size
                while (higherOrder != order) {
                    /// Mark the parent as visited, as one of its children will be occupied
                    tree_[node] = true;
                    kAssert(!tree_[node * 2 + 1], "Child of free node can't be occupied");
                    kAssert(!tree_[node * 2 + 2], "Child of free node can't be occupied");

                    /// Return the second child to the free list
                    freeLists_[higherOrder - 1].push_back(node * 2 + 2);

                    /// Go to the left node on the next level
                    node = node * 2 + 1;
                    higherOrder--;
                }
                kAssert(higherOrder == order, "We should have reached the right level");
                /// Mark the last node as busy and return it
                Logger::instance().println("Allocated node is %X", node);
                tree_[node] = true;

                // Compute physical address from node index
                return nodeToPhysicalAddress(node, order);
            }
        }

        void removeFromFreeList(size_t node, size_t order) {
            std::erase(freeLists_[order], node);
        }

        /*!
         * Free cntPages, starting from base
         * @param base The address of the first page to free
         * @param cntPages The number of pages to free
         */
        void freeImplementation(size_t base, size_t cntPages) {
            kAssert(paging::pageAligned(base), "[P_ALLOC] Address to free is not page aligned");
            size_t order = smallestPowerOf2GE(cntPages);
            size_t node = layerOffset(orderToLayer(order)) + (base - this->memBase) / (PAGE_SIZE * (1ULL << order));
            Logger::instance().println("Free node is %X", node);

            // Clear the block in the bitmap
            tree_[node] = false;

            // Clear parent blocks if they are free
            while (node != 0) {
                size_t parent_index = (node - 1) / 2;
                if (!tree_[parent_index * 2 + 1] && !tree_[parent_index * 2 + 2]) {
                    Logger::instance().println("Merging...");
                    /// Both children are free, merge them
                    tree_[parent_index] = false;
                    removeFromFreeList(getBuddyIndex(node), order);
                    node = parent_index;
                } else {
                    /// The other child node is not free
                    /// Add the current node to the free list, as it can't be merged any further
                    freeLists_[order].push_back(node);
                    break;
                }
                order++;
            }
            if (node == 0 && !tree_[0]) {
                Logger::instance().println("Pushed back root");
                freeLists_[maxOrder_].push_back(0);
            }
        }

        [[nodiscard]] constexpr static size_t neededMemoryPagesTree(size_t memSize) {
            /// For the sake of simplicity, let's take the largest power of 2 that fits the number of available pages
            auto cntPages = largestPowerOf2LE(memSize / PAGE_SIZE);

            /// We are going to need 1 + 2 + 4 + ... + cntPages nodes in our tree of used / unused pages
            /// That is 1 + 2 + 2^2 + ... + 2^x = 2^(x + 1) - 1
            size_t cntNodes = (1ULL << (cntPages + 1)) - 1;

            return toPages(cntNodes);
        }

        [[nodiscard]] constexpr static size_t neededMemoryPagesFreeLists(size_t memSize) {
            /// For the sake of simplicity, let's take the largest power of 2 that fits the number of available pages
            auto maxOrder = largestPowerOf2LE(memSize / PAGE_SIZE);

            /// We are going to need 1 + 2 + 4 + ... + cntPages nodes in our tree of used / unused pages
            /// That is 1 + 2 + 2^2 + ... + 2^x = 2^(x + 1) - 1
            size_t cntNodes = (1ULL << (maxOrder + 1)) - 1;

            /// Then we need memory for the free lists, one for each level
            return toPages(cntNodes * FREE_LIST_NODE_SIZE);
        }

        /*!
         * @return The number of pages of memory that should be mapped for the allocator
         */
        [[nodiscard]] constexpr static size_t neededMemoryPages(size_t memSize) {
            // 1 for allocation of free lists
            return neededMemoryPagesTree(memSize) + neededMemoryPagesFreeLists(memSize) + 1;
        }

    };
}
