#pragma once

#include "Genome.h"
#include "Specie.h"

#include <vector>

namespace Neat {

class Population
{
public:
	Population(size_t aCount, size_t anInputCount, size_t anOutputCount);

	size_t GetSize() const { return myGenomes.size(); }
	Genome* GetGenome(size_t aGenomeIdx) { return aGenomeIdx < myGenomes.size() ? &myGenomes[aGenomeIdx] : nullptr; }
	const Genome* GetBestGenome() const;

private:
	std::vector<Genome> myGenomes;
	std::vector<Specie> mySpecies;
};

}
