#ifndef __LEGO_LINEAR_ALLOCATOR_H__
#define __LEGO_LINEAR_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include "internal/block.h"
#include "internal/types.h"
#include "internal/pointer.h"
#include "local_allocator.h"
#include "heap_allocator.h"

//TODO
namespace lego {
	template<size_t Capacity, class Allocator>
	class LinearAllocator
	{
		char* start = nullptr;
		char* current = nullptr;
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size != 0);
			assert(alignment != 0);

			uint8_t adjustment = getAlignForwardDiff(current, alignment);

			// if not enough space, return nullptr
			if (current + adjustment + size > start + Capacity) {
				return nullptr;
			}

			// otherwise, get the aligned address
			char* alignedAddress = current + adjustment;
			current = alignedAddress + size;

			return { alignedAddress, size };
		}

		void deallocate(Blk blk) 
		{
			// does nothing
		}

		bool owns(Blk blk) const noexcept {
			return blk.ptr >= start && blk.ptr < start + Capacity;
		}

		void deallocateAll() {
			current = start;
		}


	};


	template<size_t Capacity>
	using LocalLinearAllocator = LinearAllocator<Capacity, LocalAllocator<Capacity>>;

	template<size_t Capacity>
	using HeapLinearAllocator = LinearAllocator<Capacity, HeapAllocator>;
}

#endif 