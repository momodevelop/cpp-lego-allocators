#include <iostream>
#include <vector>
#include <list>
#include "../lego/heap_allocator.h"
#include "../lego/null_allocator.h"
#include "../lego/stl_adapter.h"
#include "../lego/fallback_allocator.h"
#include "../lego/log_allocator.h"
#include "../lego/stack_allocator.h"
using namespace std;
using namespace lego;


void TestSTLOnVector() {
	using Allocator = STLAdapter<int, LogAllocator<HeapAllocator>>;
	cout << "==== TestSTLOnVector()" << endl;
	vector<int, Allocator> vec1;
	for (int i = 0; i < 10; ++i) {
		vec1.push_back(i);
	}
	for (auto&& element : vec1) {
		cout << element << " ";
	}
	cout << endl << endl;
}
void TestSTLOnList() {
	using Allocator = STLAdapter<int, LogAllocator<HeapAllocator>>;
	cout << "==== TestSTLOnList()" << endl;
	list<int, Allocator> list1;
	for (int i = 0; i < 10; ++i) {
		list1.push_back(i);
	}
	for (auto&& element : list1) {
		cout << element << " ";
	}
	cout << endl << endl;
}

void TestFallbackLocalHeap() {
	cout << "Testing Fallback/Local/Heap/Stack allocators" << endl;
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
	// Stack allocator of 100 bytes should only be able to take 50 / (4 + 1) = 10 4-byte allocations
	// +1 is due to the bookkeeping required per allocator

	for (int i = 0; i < 20; ++i) {
		if (i == 10)
			cout << "Fallback should happen here!" << endl;
		allocator.allocate(4, 4);
		
	}
}

void TestStackAllocator() {

}

int main() {
	TestSTLOnVector();
	TestSTLOnList();
	TestFallbackLocalHeap();
}