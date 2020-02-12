#ifndef __LEGO_STACKFRAME_ALLOCATOR_H__
#define __LEGO_STACKFRAME_ALLOCATOR_H__

#include "internal/block.h"
#include "internal/types.h"

namespace lego {
	class NullAllocator
	{
	public:
		Blk allocate(size_t n, uint8_t alignment) {
			return {};
		}

		void deallocate(Blk blk) {
		}

		bool owns(Blk blk) const noexcept {
			return false;
		}

		void deallocateAll() noexcept {}
	};

}

#endif