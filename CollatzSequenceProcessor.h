#ifndef COLLATZ_SEQUENCE_PROCESSOR_h
#define COLLATZ_SEQUENCE_PROCESSOR_h

#include <stdexcept>
#include <vector>
#include <mutex>
#include <atomic>
#include <syncstream>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <string>
#include <memory>

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

struct CollatzTask
{
	std::uint64_t begin;
	std::uint64_t end;
};

class CollatzSequenceProcessor
{
private:

	std::uint64_t work_portion = 0;
	std::atomic<uint64_t> nextNum{ GlobalConstants::MIN_VALUE_LIMIT };

	std::uint64_t numMax = 0;

	std::atomic<unsigned int> numOfThreads = 0;
	std::atomic<std::uint64_t> numberToOperate = 0;
	std::vector<std::thread> calculation_threads;
	std::thread main_thread;

	CollatzSequence best_result = {0,0};

	//std::unique_ptr<std::atomic<std::uint64_t>[]> cached_stash;//8bytes variant x2 memory usage
	std::unique_ptr<std::atomic<std::uint32_t>[]> cached_stash;//4bytes variant
	std::uint64_t cache_size = 0;

	mutable std::atomic<bool> is_started = false;
	mutable std::atomic<bool> is_stop = true;
	mutable std::atomic<bool> err_found = false;
	mutable std::atomic<bool> isFinito = false;

	mutable std::mutex _mt;

	//methods

	void reset();

	bool isOperationDone() const;

	void overflowTest(std::uint64_t max_n);

	void verifyNumberOfThreads(unsigned int num_of_threads, std::uint64_t max_n);

	CollatzTask createTask();

	CollatzSequence getNumSequence(std::uint64_t num) const;

	void joinCalulationThreads();

	void runThread(std::uint64_t min_num, std::uint64_t max_num);

	void runThreadOptimized();

	void runCalculation(std::uint64_t max_n, unsigned int num_of_threads);

	void runCalculationOptimized(std::uint64_t max_n, unsigned int num_of_threads);

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

	void start(std::uint64_t max_n, unsigned int threadCount);

	void stop();

	CollatzSequence getBestResult();

	bool isRunning() const;

	bool hasError() const;

	unsigned int getCurrNumOfThreads() const;

	~CollatzSequenceProcessor()
	{
		is_stop = true;

		if (main_thread.joinable())
			main_thread.join();
	}

};

#endif