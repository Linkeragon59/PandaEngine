#pragma once

#include "Genome.h"
#include "Specie.h"

#include <functional>
#include <vector>

namespace Neat {

class Population
{
public:
	Population(size_t aCount, size_t anInputCount, size_t anOutputCount);

	void TrainOneGeneration(std::function<void()> aEvaluateGenomes, std::function<void()> aGenerateOffsprings);
	void TrainGenerations(std::function<void()> aEvaluateGenomes, std::function<void()> aGenerateOffsprings, int aMaxGenerationCount, double aSatisfactionThreshold);

	size_t GetSize() const { return myGenomes.size(); }

	Genome* GetGenome(size_t aGenomeIdx) { return aGenomeIdx < myGenomes.size() ? &myGenomes[aGenomeIdx] : nullptr; }
	const Genome* GetBestGenome() const;

	std::vector<Specie>& GetSpecies() { return mySpecies; }

private:
	double GetAverageAdjustedFitness() const;
	void GroupSpecies();
	void ReplacePopulationWithOffsprings();
	std::vector<Genome> myGenomes;
	std::vector<Specie> mySpecies;
};

}
