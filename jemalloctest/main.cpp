
#include <iostream>
#include <map>
#include <chrono>
#include <windows.h>
#include <psapi.h> // after windows.h
#include <jemalloc/jemalloc.h>

/*
jemalloc
	10000000
		13918 msec.
		WorkingSetSize: 28577792
		insert calls: 5249778
		erase calls: 4750222

	100000000
		121938 msec.
		WorkingSetSize: 28569600
		insert calls: 50250207
		erase calls: 49749793

standard
	10000000
		14997 msec.
		WorkingSetSize: 27058176
		insert calls: 5249778
		erase calls: 4750222

	100000000
		139121 msec.
		WorkingSetSize: 27041792
		insert calls: 50250207
		erase calls: 49749793
*/

class Stopwatch final
{
public:

	using elapsed_resolution = std::chrono::milliseconds;

	Stopwatch()
	{
		Reset();
	}

	void Reset()
	{
		reset_time = clock.now();
	}

	elapsed_resolution Elapsed()
	{
		return std::chrono::duration_cast<elapsed_resolution>(clock.now() - reset_time);
	}

private:

	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point reset_time;
};

template <typename T>
struct MyAllocator
{
	// type definition
	typedef T value_type;

	/*	template <class U>
		struct rebind { using other = MyAllocator<U>; };*/

		// ctors
	MyAllocator() {}
	template <class T> MyAllocator(const MyAllocator<T>&) {}

	// allocate but don't initialize num elements of type T
	T* allocate(std::size_t n)
	{
		return static_cast<T*>(je_malloc(n * sizeof(T)));
	}

	// deallocate storage p of deleted elements
	void deallocate(T* p, std::size_t n)
	{
		je_free(p);
	}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename K, typename T>
using SpecMap = std::map<K, T, std::less<K>, MyAllocator<std::pair<const K, T>>>;

struct coord
{
	int x{ -1 };
	int y{ -1 };
	bool operator<(const coord& rhs) const
	{
		return x < rhs.x || (x == rhs.x && y < rhs.y);
	}
};

int main()
{
	size_t n = 100000000;
	size_t ins = 0;
	size_t rem = 0;
	//std::map<coord, int, std::less<coord>> MyMap;
	SpecMap<coord, int> MyMap;
	Stopwatch sw;
	while (n--)
	{
		int x = rand() % 1000;
		int y = rand() % 1000;
		auto it = MyMap.find({ x, y });
		if (it != MyMap.end())
		{
			MyMap.erase(it);
			++rem;
		}
		else
		{
			MyMap[{x, y}] = rand();
			++ins;
		}
	}
	std::cout << sw.Elapsed().count() << " msec." << std::endl;
	PROCESS_MEMORY_COUNTERS pmc;
	if (::GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc)))
		std::cout << "WorkingSetSize:\t" << pmc.WorkingSetSize << std::endl;
	std::cout << "insert calls: " << ins << std::endl;
	std::cout << "erase calls: " << rem << std::endl;
}
