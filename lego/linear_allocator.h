#ifndef __LEGO_HEAP_ALLOCATOR_H__
#define __LEGO_HEAP_ALLOCATOR_H__


// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	template<class Allocator>
	class LinearAllocator
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
			// does nothing
		}

		bool owns(Blk blk) const noexcept {
			return true;
		}



	};

}

#endif 