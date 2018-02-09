#include "StringTools.h"

#include <assert.h>

void AddCommasForThousand(std::string& numberString)
{
	if (numberString.find(',') != std::string::npos) { assert(false); return; }
	unsigned int stringIterLength = 3;
	while (stringIterLength < numberString.size())
	{
		numberString.insert(numberString.end() - stringIterLength, ',');
		stringIterLength += 4;
	}
}