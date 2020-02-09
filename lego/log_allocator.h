#ifndef __LEGO_LOG_ALLOCATOR_H__
#define __LEGO_LOG_ALLOCATOR_H__

// Allocator that logs to cout whenever there's allocator and deallocation (for debugging purposes)
#include "internal/block.h"
#include "internal/types.h"
#include <iostream>

namespace lego {
	class BasicLogStrategy {
	public:
		void printAllocate(Blk blk) {
			std::cout << "Allocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
		void printDeallocate(Blk blk) {
			std::cout << "Deallocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
	};


	template<class Allocator, class LogStrategy = BasicLogStrategy>
	class LogAllocator : private Allocator, private LogStrategy
	{
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			Blk ret = Allocator::allocate(n, alignment);
			LogStrategy::printAllocate(ret);
			return ret;
		}

		void deallocate(Blk blk)
		{
			LogStrategy::printDeallocate(blk);
			Allocator::deallocate(blk);
		}

		bool owns(Blk blk) const noexcept {
			return Allocator::owns(blk);
		}

		void deallocateAll()  noexcept {
			Allocator::deallocateAll();
		}
	
	};
}

#endif