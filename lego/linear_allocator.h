#ifndef __LEGO_LINEAR_ALLOCATOR_H__
#define __LEGO_LINEAR_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include "blk.h"

#include "detail/pointer.h"
#include "local_allocator.h"
#include "heap_allocator.h"

namespace lego {
	template<size_t Capacity, class Allocator>
	class LinearAllocator
	{
		static_assert(Capacity != 0);

		Allocator allocator;
		Blk memoryBlk = {};
		char* start = nullptr;
		char* current = nullptr;
	public:
		LinearAllocator() {
			memoryBlk = allocator.allocate(Capacity, alignof(max_align_t));
			assert(memoryBlk);
			start = current = reinterpret_cast<char*>(memoryBlk.ptr);

		}

		~LinearAllocator() {
			allocator.deallocate(memoryBlk);
		}

		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			uint8_t adjustment = detail::pointer::getAlignForwardDiff(current, alignment);

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
