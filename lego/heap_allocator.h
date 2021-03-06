#ifndef __LEGO_HEAP_ALLOCATOR_H__
#define __LEGO_HEAP_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.
#include <cassert>
#include "blk.h"


namespace lego {
	class HeapAllocator
	{
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);
			return { ::operator new(size),	size };
		}

		void deallocate(Blk blk)  
		{
			::operator delete(blk.ptr);
		}

		bool owns(Blk blk) const noexcept {
			return true;
		}


	};

}

#endif