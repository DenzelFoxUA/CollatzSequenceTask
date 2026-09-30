#include "CollatzSequenceProcessor.h"


void CollatzSequenceProcessor::reset()
{
	nextNum.store(GlobalConstants::MIN_VALUE_LIMIT);
	work_portion = 0;
	numOfThreads = 0;
	numberToOperate = 0;

	isFinito = false;

	std::lock_guard lock(_mt);
	numMax = 0;
	best_result = { 0,0 };
}

bool CollatzSequenceProcessor::isOperationDone() const
{
	return nextNum.load(std::memory_order_relaxed) >
		numberToOperate.load(std::memory_order_relaxed);;
}

void CollatzSequenceProcessor::overflowTest(std::uint64_t max_n)
{
	try
	{
		getNumSequence(max_n);
	}
	catch (const std::exception& ex)
	{
		std::osyncstream(std::cout) << ex.what() << "\n";
	}
}

void CollatzSequenceProcessor::verifyNumberOfThreads(unsigned int num_of_threads, std::uint64_t max_n)
{
	unsigned int maxThreads = GlobalFunctions::getHardwareNumOfThreads();

	if (max_n < GlobalConstants::MIN_VALUE_LIMIT)
		throw std::invalid_argument("Upper limit is too small");

	maxThreads = static_cast<std::uint64_t>(maxThreads) > max_n ? static_cast<unsigned int>(max_n) :
		maxThreads == 0 ? 1 : maxThreads;
	numOfThreads = std::clamp(num_of_threads, 1u, maxThreads);
}

CollatzTask CollatzSequenceProcessor::createTask()
{
	CollatzTask newTask = {};
	newTask.begin = nextNum.fetch_add(work_portion, std::memory_order_relaxed);
	newTask.end = std::min((newTask.begin + work_portion - 1),
		numberToOperate.load(std::memory_order_relaxed));

	return newTask;
}

CollatzSequence CollatzSequenceProcessor::getNumSequence(std::uint64_t num) const
{
	CollatzSequence res = {};
	res.num = num;
	res.sequence_l = 1;

	std::vector<uint64_t> path;

	while (true)
	{
		//add-on

		if (num < cache_size)
		{
			std::uint32_t length = cached_stash[num].load(std::memory_order_relaxed);

			if (length != 0)
			{

				for (auto it = path.rbegin(); it != path.rend(); ++it)
				{
					++length;

					if (*it < cache_size)
						cached_stash[*it].store(length, std::memory_order_relaxed);
				}
			}

			res.sequence_l = length;

			return res;
		}

		path.push_back(num);

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
	}
}

void CollatzSequenceProcessor::joinCalulationThreads()
{
	for (auto& thr : calculation_threads)
	{
		if (thr.joinable())
			thr.join();
	}

	calculation_threads.clear();

}

void CollatzSequenceProcessor::runThread(std::uint64_t min_num, std::uint64_t max_num)
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

		for (std::uint64_t min = min_num + 1; min <= max_num; min++)
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

void CollatzSequenceProcessor::runThreadOptimized()
{
	std::thread nums_pool_thread([this]() {

		while (!isFinito.load(std::memory_order_relaxed) &&
			!is_stop.load(std::memory_order_relaxed) &&
			!err_found.load(std::memory_order_relaxed))
		{
			CollatzTask _task = createTask();

			isFinito.store(isOperationDone());

			CollatzSequence local_val;

			for (std::uint64_t min = _task.begin; min <= _task.end; min++)
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

		};
		});

	std::lock_guard lock(_mt);
	calculation_threads.push_back(std::move(nums_pool_thread));
}

void CollatzSequenceProcessor::runCalculation(std::uint64_t max_n, unsigned int num_of_threads)
{
	verifyNumberOfThreads(num_of_threads, max_n);
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

void CollatzSequenceProcessor::runCalculationOptimized(std::uint64_t max_n, unsigned int num_of_threads)
{
	verifyNumberOfThreads(num_of_threads, max_n);
	numMax = max_n;

	work_portion = std::min<uint64_t>(numMax / numOfThreads, GlobalConstants::MAX_WORK_PORTION);

	calculation_threads.reserve(numOfThreads);

	is_started = true;

	for (unsigned int i = 0; i < numOfThreads; i++)
	{
		runThreadOptimized();
	}

}

void CollatzSequenceProcessor::start(std::uint64_t max_n, unsigned int threadCount)
{
	if (!is_started)
	{
		if (main_thread.joinable())
			main_thread.join();

		reset();

		is_stop = false;
		err_found = false;

		is_started = true;

		//changes
		nextNum.store(GlobalConstants::MIN_VALUE_LIMIT);
		numberToOperate.store(max_n);

		cache_size = max_n + 1;
		cached_stash = std::make_unique<std::atomic<std::uint32_t>[]>(cache_size);

		for (std::uint64_t i = 0; i < cache_size; ++i)
		{
			cached_stash[i].store(0, std::memory_order_relaxed);
		}
		cached_stash[1].store(1, std::memory_order_relaxed);
		//

		std::thread run_main([this, max_n, threadCount]() {

			//runCalculation(max_n, threadCount);

			overflowTest(max_n);

			runCalculationOptimized(max_n, threadCount);

			joinCalulationThreads();

			is_started = false;
			});

		main_thread = std::move(run_main);
	}

}

void CollatzSequenceProcessor::stop()
{
	if (is_started)
	{
		std::osyncstream(std::cout) << " <<<<STOP>>>>> Operation stoped!\n";

		is_stop = true;
	}

}


CollatzSequence CollatzSequenceProcessor::getBestResult()
{
	if (main_thread.joinable())
		main_thread.join();

	std::lock_guard lock(_mt);
	return best_result;
}

bool CollatzSequenceProcessor::isRunning() const
{
	return is_started.load();
}

bool CollatzSequenceProcessor::hasError() const
{
	return err_found.load();
}

unsigned int CollatzSequenceProcessor::getCurrNumOfThreads() const
{
	return numOfThreads;
}