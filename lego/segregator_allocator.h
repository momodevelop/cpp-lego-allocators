#ifndef __LEGO_SEGREGATOR_ALLOCATOR_H__
#define __LEGO_SEGREGATOR_ALLOCATOR_H_

#include <cassert>
#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	template <size_t Threshhold, class SmallAllocator, class BigAllocator>
	class SegregatorAllocator 
	{
		SmallAllocator smallAllocator;
		BigAllocator bigAllocator;
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			if (n > Threshhold) {
				return bigAllocator.allocate(n, alignment);
			}
			else {
				return smallAllocator.allocate(n, alignment);
			}
		}

		void deallocate(Blk blk) noexcept  // Use pointer if pointer is not a value_type*
		{
			if (smallAllocator.owns(blk))
				smallAllocator.deallocate(blk);
			else if (bigAllocator.owns(blk))
				bigAllocator.deallocate(blk);
			else
				assert(false);
		}

		bool owns(const Blk& blk) const {
			return smallAllocator.owns(blk) || bigAllocator.owns(blk);
		}



	};
}
#endif


