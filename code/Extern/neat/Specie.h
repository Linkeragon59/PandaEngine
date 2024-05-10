#pragma once

#include <vector>

namespace Neat {

class Genome;

class Specie
{
public:
	size_t GetSize() const { return myGenomes.size(); }
	bool BelongsToSpecie(const Genome* aGenome) const;

	void GenerateOffsprings();

private:
	size_t ComputeNextSize(double anAverageFitness) const;

	std::vector<Genome*> myGenomes;
	std::vector<Genome> myOffsprings;
};

}
