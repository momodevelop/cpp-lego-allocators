#ifndef __LEGO_HEAP_ALLOCATOR_H__
#define __LEGO_HEAP_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	class HeapAllocator
	{
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			return { 
				::operator new(n), 
				n 
			};
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