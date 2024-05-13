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
	~Population();

	struct TrainingCallbacks
	{
		std::function<void()> myOnTrainGenerationStart;
		std::function<void()> myOnTrainGenerationEnd; // Exposed for parallelization
		
		std::function<void()> myEvaluateGenomes;
	};
	void TrainOneGeneration(const TrainingCallbacks& someCallbacks);
	void TrainGenerations(const TrainingCallbacks& someCallbacks, int aMaxGenerationCount, double aSatisfactionThreshold);

	size_t GetSize() const { return myGenomes.size(); }

	Genome* GetGenome(size_t aGenomeIdx) { return aGenomeIdx < myGenomes.size() ? &myGenomes[aGenomeIdx] : nullptr; }
	const Genome* GetBestGenome() const;

	std::vector<Specie*>& GetSpecies() { return mySpecies; }

private:
	double GetAverageAdjustedFitness() const;
	void GroupSpecies();
	void ReplacePopulationWithOffsprings();
	std::vector<Genome> myGenomes;
	std::vector<Specie*> mySpecies;
};

}
