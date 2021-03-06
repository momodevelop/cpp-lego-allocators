#include <iostream>
#include <vector>
#include <list>
#include "../lego/heap_allocator.h"
#include "../lego/null_allocator.h"
#include "../lego/stl_adapter.h"
#include "../lego/fallback_allocator.h"
#include "../lego/log_allocator.h"
#include "../lego/stack_allocator.h"
#include "../lego/linear_allocator.h"
#include "../lego/segregator_allocator.h"
#include "../lego/freelist_allocator.h"
#include "../lego/slab_allocator.h"

using namespace std;
using namespace lego;


void TestSTLOnVector() {
	using Allocator = STLAdapter<int, HeapAllocator>;
	cout << "==== TestSTLOnVector()" << endl;
	vector<int, Allocator> vec1;
	for (int i = 0; i < 10; ++i) {
		vec1.push_back(i);
	}
	bool green = true;
	int i = 0;
	for (auto&& element : vec1) {
		if (element != i++) {
			green = false;
			break;
		}
	}
	cout << "Testing..." << (green ? "YES!" : "NO!") << endl;
	cout << endl;
}
void TestSTLOnList() {
	using Allocator = STLAdapter<int, HeapAllocator>;
	cout << "==== TestSTLOnList()" << endl;
	list<int, Allocator> list1;
	for (int i = 0; i < 10; ++i) {
		list1.push_back(i);
	}

	bool green = true;
	int i = 0;
	for (auto&& element : list1) {
		if (element != i++) {
			green = false;
			break;
		}
	}
	cout << "Testing..." << (green ? "YES!" : "NO!") << endl;
	cout << endl;
}

void TestFallbackLocalHeap() {
	cout << "=== Testing Fallback/Local/Heap/Stack allocators" << endl;
	struct StackLogStrategy {
		void printAllocate(Blk blk) {
			std::cout << "Stack Allocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
		void printDeallocate(Blk blk) {
			std::cout << "Stack Deallocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
	};


	struct MallocLogStrategy {
		void printAllocate(Blk blk) {
			std::cout << "Malloc Allocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
		void printDeallocate(Blk blk) {
			std::cout << "Malloc Deallocating: " << blk.size << " @ " << blk.ptr << std::endl;
		}
	};

	using Allocator = FallbackAllocator<
		LogAllocator<LocalStackAllocator<50>, StackLogStrategy>,
		LogAllocator<HeapAllocator, MallocLogStrategy>>;

	Allocator allocator;
	// Stack allocator of 50 bytes should only be able to take 50 / (4 + 1) = 10 4-byte allocations
	// +1 is due to the bookkeeping required per allocator

	for (int i = 0; i < 20; ++i) {
		if (i == 10)
			cout << "Fallback should happen here!" << endl;
		allocator.allocate(4, 4);

	}

	cout << endl;
}

void TestStackAllocator() {
	cout << "=== Testing Stack allocator" << endl;
	// Stack allocator of 100 bytes should only be able to take 100 / (4 + 1) = 20 4-byte allocations
	// +1 is due to the bookkeeping required per allocator
	using Allocator = HeapStackAllocator<100>;
	Allocator allocator;

	auto startBlk = allocator.allocate(4, 4); // store the memory address of first item's allocation
	allocator.deallocate(startBlk);

	vector<Blk> blkList;
	bool allocatesSuccess = true;
	for (int i = 0; i < 20; ++i) {
		auto blk = allocator.allocate(4, 4);
		blkList.push_back(blk);
		if (!blk)
			allocatesSuccess = false;
		
	}
	auto expectNullBlk = allocator.allocate(4, 4);
	cout << "Testing allocate integrity..." << (!expectNullBlk && allocatesSuccess ? "YES" : "NO") << endl;

	// deallocate test
	for (int i = 19; i >= 0; --i) {
		allocator.deallocate(blkList[i]);
	}
	auto tmpBlk = allocator.allocate(4, 4);
	cout << "Testing deallocate integrity..." << (tmpBlk == startBlk ? "YES" : "NO") << endl;
	allocator.deallocate(tmpBlk);

	//deallocate all test
	for (int i = 0; i < 20; ++i) {
		blkList[i] = allocator.allocate(4, 4);
	}
	allocator.deallocateAll();
	tmpBlk = allocator.allocate(4, 4);
	allocator.deallocate(tmpBlk);

	cout << "Testing deallocateAll integrity..." << (tmpBlk == startBlk ? "YES" : "NO") << endl;

	cout << endl;
}

void TestLinearAllocator() {
	cout << "=== Testing Linear allocator" << endl;
	using Allocator = HeapLinearAllocator<100>;
	Allocator allocator;

	bool allocatesSuccess = true;
	for (int i = 0; i < 25; ++i) {
		auto blk = allocator.allocate(4, 4);
		if (!blk)
			allocatesSuccess = false;
	}
	auto expectNullBlk = allocator.allocate(4, 4);
	cout << "Testing allocate integrity..." << (!expectNullBlk && allocatesSuccess ? "YES" : "NO") << endl;
	allocator.deallocateAll();

	allocatesSuccess = true;
	for (int i = 0; i < 25; ++i) {
		auto blk = allocator.allocate(4, 4);
		if (!blk)
			allocatesSuccess = false;
	}
	expectNullBlk = allocator.allocate(4, 4);
	cout << "Testing deallocateAll integrity..." << (!expectNullBlk && allocatesSuccess ? "YES" : "NO") << endl;
	cout << endl;
}
void TestSegregatorAllocator() {
	cout << "=== Testing SegregatorAllocator" << endl;
	using Allocator = SegregatorAllocator<4,
			LocalLinearAllocator<100>,
			LocalLinearAllocator<100>
	>;

	Allocator allocator;

	// the memory given back on the first allocation of the 
	// 'small' LocalLinearAllocator should be at least 100 bytes after the
	// first allocation of the 'big' LocalLinearAllocator.

	auto small = allocator.allocate(4, 4);
	auto large = allocator.allocate(8, 4);
	cout << "Testing allocate integrity..." << (((char*)large.ptr - (char*)small.ptr ) >= 100 ? "YES" : "NO") << endl;
	cout << endl;

}


void TestFreeListAllocator() {
	cout << "=== Testing FreeListAllocator" << endl;

	using Allocator = LocalFirstFitFreeListAllocator<1000>;
	// We'll do a simple test. Allocate and deallocate semi-randomly.
	// Once everything is deallocated, next allocation should be same as first allocation.
	Allocator allocator;
	auto blk = allocator.allocate(4, 4);
	const char* const firstAllocationPtr = reinterpret_cast<char*>(blk.ptr);
	allocator.deallocate(blk);

	Blk blks[20];
	for (size_t i = 0; i < 20; ++i) {
		blks[i] = allocator.allocate(i + 1, (uint8_t)(pow(2.0, (double)(i % 4))));
		if (!blks[i]) {
			cout << "Failed to allocate..." << endl;
			return;
		}

		(*(char*)((blks[i]).ptr)) = 'A';

		if (i % 4 == 0) {
			allocator.deallocate(blks[i / 2]);
			blks[i / 2] = {};
		}
	}

	for (int i = 0; i < 20; ++i) {
		allocator.deallocate(blks[i]);
	}
	blk = allocator.allocate(4, 4);
	cout << "Testing allocate integrity..." << ((char*)blk.ptr == firstAllocationPtr ? "YES" : "NO") << endl;
	cout << endl;
}


void TestFreeListFirstFitAllocator() {
	cout << "=== Testing FreeListFirstFitAllocator" << endl;

	using Allocator = LocalFirstFitFreeListAllocator<1000>;
	// We'll do a simple test. Allocate and deallocate semi-randomly.
	// Once everything is deallocated, next allocation should be same as first allocation.
	Allocator allocator;
	auto blk1 = allocator.allocate(4, 4);
	auto blk2 = allocator.allocate(4, 4);
	auto blk3 = allocator.allocate(4, 4);
	auto blk4 = allocator.allocate(4, 4);

	allocator.deallocate(blk2);
	auto blk5 = allocator.allocate(4, 4);
	cout << "Testing best fit integrity..." << ((blk5.ptr < blk4.ptr && blk5.ptr > blk1.ptr) ? "YES" : "NO") << endl;
	cout << endl;
}

void TestFreeListBestFitAllocator() {
	cout << "=== Testing FreeListBestFitAllocator" << endl;

	using Allocator = LocalBestFitFreeListAllocator<1000>;
	// We'll do a simple test. Allocate and deallocate semi-randomly.
	// Once everything is deallocated, next allocation should be same as first allocation.
	Allocator allocator;
	auto blk1 = allocator.allocate(4, 4);
	auto blk2 = allocator.allocate(10, 4);
	auto blk3 = allocator.allocate(4, 4);
	auto blk4 = allocator.allocate(4, 4);
	auto blk5 = allocator.allocate(4, 4);

	allocator.deallocate(blk2);
	allocator.deallocate(blk4);
	auto blk6 = allocator.allocate(4, 4);
	cout << "Testing best fit integrity..." << ((blk6.ptr > blk3.ptr&& blk6.ptr < blk5.ptr) ? "YES" : "NO") << endl;
	cout << blk3.ptr << endl;
	cout << blk6.ptr << endl;
	cout << blk5.ptr << endl;
}

void TestSlabAllocator() {
	cout << "=== Testing SlabAllocator" << endl;
	using Allocator = SlabAllocator<1000, 25, 4, LocalAllocator<1000>>;
	Allocator allocator;


}

int main() {
	TestSTLOnVector();
	TestSTLOnList();
	TestFallbackLocalHeap();
	TestStackAllocator();
	TestLinearAllocator();
	TestSegregatorAllocator();
	TestFreeListAllocator();
	TestFreeListFirstFitAllocator();
	TestFreeListBestFitAllocator();
	TestSlabAllocator();
}