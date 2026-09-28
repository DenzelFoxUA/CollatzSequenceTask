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
	static constexpr const int NUM_OF_DIGITS_MAX = 20;
	static constexpr std::uint64_t MIN_VALUE_LIMIT = 1;
	static constexpr std::uint64_t MAX_VALUE_LIMIT = 100000000;
	static constexpr std::uint64_t CALC_DEFAULT_NUMBER = 10000000;

	static constexpr const int DEFAULT_TIMER_INTERVAL_MSEC = 50;

	static constexpr std::uint64_t MAX_WORK_PORTION = 1000;
}

namespace RegExPatterns
{
	static constexpr const char* NUM_INPUT = "[0-9]{1,20}";
}

#endif
