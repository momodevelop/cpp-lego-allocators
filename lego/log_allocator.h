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
	class LogAllocator
	{
		Allocator allocator;
		LogStrategy logStrategy;
	public:
		Blk allocate(size_t n, uint8_t alignment)
		{
			Blk ret = allocator.allocate(n, alignment);
			logStrategy.printAllocate(ret);
			return ret;
		}

		void deallocate(Blk blk)
		{
			logStrategy.printDeallocate(blk);
			allocator.deallocate(blk);
		}

		bool owns(Blk blk) const noexcept {
			return allocator.owns(blk);
		}

		void deallocateAll() noexcept {
			allocator.deallocateAll();
		}
	
	};
}

#endif