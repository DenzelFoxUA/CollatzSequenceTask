#ifndef CONSTANTS_h
#define CONSTANTS_h

#include <thread>
#include <cstdint>
#include <limits>

namespace GlobalFunctions
{
	static unsigned int getHardwareNumOfThreads()
	{
		return std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();
	}
}

namespace GlobalConstants
{
	static constexpr std::uint64_t MAX_VALUE_LIMIT = 10000000;
}


#endif
