#include "Population.h"

namespace Neat {

Population::Population(size_t aCount, size_t anInputCount, size_t anOutputCount)
{
	myGenomes.reserve(aCount);
	for (size_t i = 0; i < aCount; ++i)
		myGenomes.push_back(Genome(anInputCount, anOutputCount));
}

const Genome* Population::GetBestGenome() const
{
	const Genome* bestGenome = nullptr;
	double bestFitness = -DBL_MAX;
	for (const Genome& genome : myGenomes)
	{
		if (genome.GetFitness() > bestFitness)
		{
			bestGenome = &genome;
			bestFitness = genome.GetFitness();
		}
	}
	return bestGenome;
}

}
