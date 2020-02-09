#ifndef __LEGO_FALLBACK_ALLOCATOR_H__
#define __LEGO_FALLBACK_ALLOCATOR_H__

// If the Primary allocator fails, the Fallback allocator will allocate.

#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	template <class Primary, class Fallback>
	class FallbackAllocator : private Primary, private Fallback
	{
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			Blk blk = Primary::allocate(n, alignment);
			if (!blk) {
				return Fallback::allocate(n, alignment);
			}
	
			return blk;
		}

		void deallocate(Blk blk) noexcept  // Use pointer if pointer is not a value_type*
		{
			if (Primary::owns(blk))
				Primary::deallocate(blk);
			else
				Fallback::deallocate(blk);
		}

		bool owns(const Blk& blk) const {
			return Primary::owns(blk) || Fallback::owns(blk);
		}



	};
}


#endif