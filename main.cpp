#include "CollatzSequenceProcessor.h"



int main()
{
	std::uint64_t num = 6148914691236517203;

	CollatzSequenceProcessor& csp = CollatzSequenceProcessor::getInstance();
	csp.start(562200, 6);
	//csp.stop();
	//std::this_thread::sleep_for(std::chrono::seconds(5));
	CollatzSequence l_seq = csp.getBestResult();

	
	

	std::cout << "Best num -> " + std::to_string(l_seq.num) + " Sequence = " + std::to_string(l_seq.sequence_l);
}