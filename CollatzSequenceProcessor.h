#ifndef COLLATZ_SEQUENCE_PROCESSOR_h
#define COLLATZ_SEQUENCE_PROCESSOR_h


#include <stdexcept>
#include <cstring>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <syncstream>
#include <iostream>
#include <chrono>
#include <algorithm>

#include "Constants.h"

struct CollatzSequence
{
public:
	std::uint64_t num = 0;
	size_t sequence_l = 0;

	CollatzSequence() = default;
	CollatzSequence(std::uint64_t _num, size_t _lenght) : num(_num), sequence_l(_lenght) {}

	bool operator >= (const CollatzSequence& other) const
	{
		return sequence_l >= other.sequence_l;
	}

	bool operator <= (const CollatzSequence& other) const
	{
		return sequence_l <= other.sequence_l;
	}


	CollatzSequence(const CollatzSequence& other)
	{
		num = other.num;
		sequence_l = other.sequence_l;
	}

	CollatzSequence& operator=(const CollatzSequence& other)
	{
		if (this != &other)
		{
			num = other.num;
			sequence_l = other.sequence_l;
		}
		return *this;
	}

	CollatzSequence(CollatzSequence&& other) noexcept = default;
	CollatzSequence& operator=(CollatzSequence&& other) noexcept = default;
};

class CollatzSequenceProcessor
{
private:
	static constexpr std::uint64_t firstNum = 1;
	std::uint64_t numMax = 0;

	std::atomic<unsigned int> numOfThreads = 0;
	std::vector<std::thread> calculation_threads;
	std::thread main_thread;

	CollatzSequence best_result = {0,0};

	mutable std::atomic<bool> is_started = false;
	mutable std::atomic<bool> is_stop = true;
	mutable std::atomic<bool> err_found = false;

	mutable std::mutex _mt;

	//methods

	void reset()
	{
		numOfThreads = 0;

		std::lock_guard lock(_mt);
		numMax = 0;
		best_result = { 0,0 };
	}

	CollatzSequence getNumSequence(std::uint64_t num) const
	{
		CollatzSequence res = {};
		res.num = num;
		res.sequence_l = 1;

		while (num > firstNum)
		{
			if (num % 2 == 0)
			{
				num /= 2;
			}
			else
			{
				if ((std::numeric_limits<std::uint64_t>::max() - 1) / 3 < num)
				{
					err_found.store(true);

					throw std::overflow_error("Number: " + std::to_string(res.num) +
						"\nERR_THROWN::ON->" + std::to_string(num) + " SEQUENCE OVERFLOW");

				}

				num = 3 * num + 1;
			}
			res.sequence_l++;
		}

		return res;
	}

	void joinCalulationThreads()
	{
		for (auto& thr : calculation_threads)
		{
			if (thr.joinable())
				thr.join();
		}

		calculation_threads.clear();

	}

	void runThread(std::uint64_t min_num, std::uint64_t max_num)
	{
		std::thread nums_pool_thread([this, min_num, max_num]() {

			CollatzSequence local_val;

			if (!is_stop && !err_found)
			{
				try
				{
					local_val = getNumSequence(min_num);
				}
				catch (const std::exception& ex)
				{
					std::osyncstream(std::cout) << ex.what() << "\n";
				}
			}
			
			for (std::uint64_t min = min_num+1; min <= max_num; min++)
			{

				if (is_stop || err_found)
					break;

				CollatzSequence buffer = {};
				try
				{
					buffer = getNumSequence(min);
				}
				catch (const std::exception& ex)
				{
					std::osyncstream(std::cout) << ex.what() << "\n";
				}

				local_val = buffer >= local_val ? std::move(buffer) : local_val;

			}

			if (!is_stop && !err_found)
			{
				std::lock_guard lock(_mt);
				best_result = local_val >= best_result ? std::move(local_val) : best_result;
			}

		});

		std::lock_guard lock(_mt);
		calculation_threads.push_back(std::move(nums_pool_thread));
	}

	void runCalculation(std::uint64_t max_n, unsigned int num_of_threads)
	{
		unsigned int maxThreads = GlobalFunctions::getHardwareNumOfThreads();
		maxThreads = static_cast<std::uint64_t>(maxThreads) > max_n ? 
			static_cast<unsigned int>(max_n) : maxThreads;
		numOfThreads = std::clamp(num_of_threads, 1u, maxThreads);
		numMax = max_n;
		calculation_threads.reserve(numOfThreads);

		is_started = true;

		std::uint64_t step = max_n / static_cast<std::uint64_t>(numOfThreads);
		std::uint64_t remain = max_n % static_cast<std::uint64_t>(numOfThreads);
		std::uint64_t min = 1, max;

		for (unsigned int i = 0; i < numOfThreads; i++)
		{
			std::uint64_t total_step = step + (i < remain ? 1 : 0);
			max = min + total_step - 1;

			runThread(min, max);

			min = max + 1;
		}

	}

	CollatzSequenceProcessor() = default;

public:

	CollatzSequenceProcessor(const CollatzSequenceProcessor&) = delete;
	CollatzSequenceProcessor& operator=(const CollatzSequenceProcessor&) = delete;

	CollatzSequenceProcessor(CollatzSequenceProcessor&&) = delete;
	CollatzSequenceProcessor& operator=(CollatzSequenceProcessor&&) = delete;

	//-----------------------------------------
	
	static CollatzSequenceProcessor& getInstance()
	{
		static CollatzSequenceProcessor instance = CollatzSequenceProcessor();
		return instance;
	}

	void start(std::uint64_t max_n, unsigned int threadCount)
	{
		if (!is_started)
		{
			if (main_thread.joinable())
				main_thread.join();

			reset();

			is_stop = false;
			err_found = false;
			
			is_started = true;

			std::thread run_main([this, max_n,threadCount]() {

				runCalculation(max_n, threadCount);

				joinCalulationThreads();

				is_started = false;
			});

			main_thread = std::move(run_main);
		}

	}

	void stop()
	{
		if (is_started)
		{
			std::osyncstream(std::cout) << " <<<<STOP>>>>> Operation stoped!\n";

			is_stop = true;
		}

	}


	CollatzSequence getBestResult()
	{
		if (main_thread.joinable())
			main_thread.join();

		std::lock_guard lock(_mt);
		return best_result;
	}

	bool isRunning() const
	{
    	return is_started.load();
	}

	bool hasError() const
	{
    	return err_found.load();
	}

	unsigned int getCurrNumOfThreads() const
	{
		return numOfThreads;
	}

	~CollatzSequenceProcessor()
	{
		is_stop = true;

		if (main_thread.joinable())
			main_thread.join();
	}

};

#endif