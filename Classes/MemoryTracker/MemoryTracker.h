#ifndef _MEMORYTRACKER_H_
#define _MEMORYTRACKER_H_

#include <string>

void TrackAllocate(std::string allocation_type, const int amount);
void TrackDeallocate(std::string allocation_type, const int amount);
void OutputMemoryTracking();

#endif // _MEMORYTRACKER_H_