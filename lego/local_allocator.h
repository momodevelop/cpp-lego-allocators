#ifndef __LEGO_LOCAL_ALLOCATOR_H__
#define __LEGO_LOCAL_ALLOCATOR_H__

// Simple allocator that reserves memory on the stack frame.
// It's basically a wrapper around an array.
// It will always return the start point of the array on 'allocation'.
// Once it's allocated, cannot allocate again until it's deallocated.
// This is useful if you do not want to reserve memory in the heap using the HeapAllocator


#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	template<size_t Capacity>
	class LocalAllocator
	{
		char arr[Capacity] = {0};
		bool allocated = false;
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			if (allocated || n > Capacity)
				return {};
			else {
				allocated = true;
				return { arr, n };
			}
		}

		void deallocate(Blk blk)  
		{
			if (owns(blk) && allocated)
				allocated = false;
		}

		bool owns(Blk blk) const noexcept {
			return blk.ptr == arr;
		}

		void deallocateAll()
		{
			deallocate();
		}
	};

}

#endif