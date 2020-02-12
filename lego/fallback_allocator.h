#ifndef __LEGO_FALLBACK_ALLOCATOR_H__
#define __LEGO_FALLBACK_ALLOCATOR_H__

// If the Primary allocator fails, the Fallback allocator will allocate.

#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	template <class Primary, class Fallback>
	class FallbackAllocator
	{
		Primary primary;
		Fallback fallback;
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			Blk blk = primary.allocate(n, alignment);
			if (!blk) {
				return fallback.allocate(n, alignment);
			}
	
			return blk;
		}

		void deallocate(Blk blk) noexcept  // Use pointer if pointer is not a value_type*
		{
			if (primary.owns(blk))
				primary.deallocate(blk);
			else
				fallback.deallocate(blk);
		}

		bool owns(const Blk& blk) const {
			return primary.owns(blk) || fallback.owns(blk);
		}



	};
}


#endif