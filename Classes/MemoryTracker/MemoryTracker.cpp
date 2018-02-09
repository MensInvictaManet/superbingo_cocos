#include "MemoryTracker.h"

//#define MEMORY_TRACKING
#ifdef MEMORY_TRACKING

	#include <map>
	#include <fstream>
	std::map< std::string, int > AllocationList;
	int OverallMemoryUsage = 0;

	void TrackAllocate(std::string allocation_type, const int amount)
	{
		if (AllocationList.find(allocation_type) == AllocationList.end()) AllocationList[allocation_type] = 0;
		AllocationList[allocation_type] += amount;
		OverallMemoryUsage += amount;
	}

	void TrackDeallocate(std::string allocation_type, const int amount)
	{
		if (AllocationList.find(allocation_type) == AllocationList.end()) AllocationList[allocation_type] = 0;
		AllocationList[allocation_type] -= amount;
		OverallMemoryUsage -= amount;
	}

	void OutputMemoryTracking()
	{
		std::ofstream output_file("memory_tracking.txt");
		if (output_file.bad() || !output_file.good()) { return; }

		output_file << "Total Memory Usage:   " << OverallMemoryUsage << std::endl << std::endl;

		for (std::map< std::string, int >::const_iterator iter = AllocationList.begin(); iter != AllocationList.end(); ++iter)
		{
			output_file << (*iter).first << ":   " << (*iter).second << std::endl;
		}

		output_file.close();
	}
#else
	void TrackAllocate(std::string, const int) {}
	void TrackDeallocate(std::string, const int) {}
	void OutputMemoryTracking() {}
#endif