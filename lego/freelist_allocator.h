#ifndef __LEGO_FREELIST_ALLOCATOR_H__
#define __LEGO_FREELIST_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include "internal/block.h"
#include "internal/types.h"
#include "internal/pointer.h"
#include "local_allocator.h"
#include "heap_allocator.h"

namespace lego {
	template<size_t Capacity, class Allocator>
	class FreeListAllocator {
		static_assert(Capacity != 0);
	protected:
		union FreeBlock {
			struct {
				size_t size;
				FreeBlock* next;
			} block;
		protected:
			max_align_t Align;
		};

		union Header {
			size_t size;
		protected:
			max_align_t Align;
		};

		Blk memory = {};
		char* start = nullptr;
		FreeBlock* freeList = nullptr;
		Allocator allocator;

		constexpr size_t MinBlockSize() {
			return sizeof(Header) > sizeof(FreeBlock) ? sizeof(Header) : sizeof(FreeBlock);
		}

	public:
		FreeListAllocator()
		{
			memory = allocator.allocate(Capacity, alignof(max_align_t));
			assert(memory);
			start = static_cast<char*>(memory.ptr);

			deallocateAll();

			// The whole allocator must be able to contain at least a minimum block size
			assert(freeList->block.size > MinBlockSize());
		}


		~FreeListAllocator()
		{
			allocator.deallocate(memory);
		}


		Blk allocate(size_t size, uint8_t alignment) {
			assert(size && alignment);

			// Calculate the size of the header + object rounded to alignment.
			// All our objects and headers will be aligned to the maximum alignment size. 
			size_t roundObjectSize = pointer::roundToAlignment(size, alignof(max_align_t));
			size_t totalSize = sizeof(Header) + roundObjectSize;

			// Search for a free block that fits the size.
			FreeBlock* itr = freeList;
			FreeBlock* prev = nullptr;
			while (itr != nullptr) {

				// We don't have to calculate any adjustment because our start is aligned to max_size_t and every
				// object is aligned to rounded to max_size_t

				// if the allocation fits, break.
				if (totalSize <= itr->block.size) {
					break;
				}

				// otherwise, go to the next FreeBlock
				prev = itr;
				itr = itr->block.next;
			}

			// Could not find a block that fits
			if (itr == nullptr)
				return {};

			// Here, we have found a block that fits and update our freeList.
			// Check if the block can be split after allocation.
			// It can be split if after allocation, it can store more than sizeof(Header)
			// Note: We don't have to calculate adjustment for the 'future' Header 
			// because our object and Header sizes are rounded to maximum alignment, so the adjustment
			// is already within totalSize.

			FreeBlock* nextBlock;
			size_t remainingSize = itr->block.size - totalSize;
			if (remainingSize <= MinBlockSize()) {
				// if it's smaller or equal to the rounded Header size, 
				// future allocations in this block is impossible.

				// The next block would then be itr->next
				nextBlock = itr->block.next;

				// let the total size be the whole block
				totalSize = itr->block.size;
			}

			else {
				// Otherwise, create a new FreeBlock after the current block
				nextBlock = reinterpret_cast<FreeBlock*>(pointer::add(itr, totalSize));
				nextBlock->block.size = remainingSize;
				nextBlock->block.next = itr->block.next;

			}

			// Here, we update our linked list!
			// If there's a previous block, set it's next to itr->next
			if (prev) {
				prev->block.next = nextBlock;
			}
			// If there is no previous block, it means that itr is the head.
			// Set the head to the next block
			else {
				this->freeList = nextBlock;
			}

			// Get and Update the header
			Header* header = reinterpret_cast<Header*>(itr);
			header->size = totalSize;

			// Get the object to return to the user
			void* ret = pointer::add(itr, sizeof(Header));
			return { ret, size };
		}

		bool owns(Blk blk) const noexcept {
			return reinterpret_cast<char*>(blk.ptr) >= start && reinterpret_cast<char*>(blk.ptr) < start + Capacity;
		}

		// Reset all variables to start
		void deallocateAll() noexcept {
			this->freeList = reinterpret_cast<FreeBlock*>(start);
			this->freeList->block.size = Capacity;
			this->freeList->block.next = nullptr;
		}

		void deallocate(Blk blk)
		{
			if (!blk)
				return;

			assert(owns(blk));

			Header* header = reinterpret_cast<Header*>(pointer::sub(blk.ptr, sizeof(Header)));
			uintptr_t blockEnd = reinterpret_cast<uintptr_t>(pointer::add(header, header->size));

			// Look for a FreeBlock which we can combine
			FreeBlock* itr = freeList;
			FreeBlock* prev = nullptr;
			while (itr != nullptr) {
				// search until we are past the current block
				if (reinterpret_cast<uintptr_t>(itr) >= blockEnd)
					break;
				prev = itr;
				itr = itr->block.next;
			}

			// if there is no prev block, that means itr is at the start of the block
			// set the head of freeList to be this block.
			if (prev == nullptr) {
				// use prev for the next step of combining with the next block
				prev = reinterpret_cast<FreeBlock*>(header);
				prev->block.size = header->size;
				prev->block.next = freeList;
				freeList = prev;
			}

			else if (reinterpret_cast<uintptr_t>(prev) + prev->block.size == reinterpret_cast<uintptr_t>(header)) {
				// If the previous block is directly next to this block, combine by just adding this block's size 
				// to the prev block's size
				prev->block.size += header->size;
			}

			else {
				// Here, there is a prev block, but it is not next to the current block, so we can't combine
				// So we turn the current block into a FreeBlock.
				FreeBlock* temp = reinterpret_cast<FreeBlock*>(header);
				// a bit of a tightrope here, but it should be fine because we have not touched the memory around header->size
				temp->block.size = header->size;
				temp->block.next = prev->block.next; // now, header->size is gone.

				// update prev block
				prev->block.next = temp;

				// set this for the next step of combining with the next block
				prev = temp;
			}

			// Check if we can combine prev with the NEXT block
			if (itr != nullptr && reinterpret_cast<uintptr_t>(itr) == blockEnd)
			{
				prev->block.size += itr->block.size;
				prev->block.next = itr->block.next;
			}
		}
	};


	template<size_t Capacity>
	using LocalFreeListAllocator = FreeListAllocator<Capacity, LocalAllocator<Capacity>>;

	template<size_t Capacity>
	using HeapFreeListAllocator = FreeListAllocator<Capacity, HeapAllocator>;
}

#endif 
