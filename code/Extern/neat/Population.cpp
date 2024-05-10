#include "Population.h"

namespace Neat {

Population::Population(size_t aCount, size_t anInputCount, size_t anOutputCount)
{
	myGenomes.reserve(aCount);
	for (size_t i = 0; i < aCount; ++i)
		myGenomes.push_back(Genome(anInputCount, anOutputCount));
}



}
