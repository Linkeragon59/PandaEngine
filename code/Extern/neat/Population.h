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

	size_t GetSize() const { return myGenomes.size(); }
	
	struct TrainingCallbacks
	{
		std::function<void()> myOnTrainGenerationStart;
		std::function<void()> myOnTrainGenerationEnd; // Exposed for parallelization
		
		std::function<void()> myEvaluateGenomes;
	};
	void TrainOneGeneration(const TrainingCallbacks& someCallbacks);
	void TrainGenerations(const TrainingCallbacks& someCallbacks, int aMaxGenerationCount, double aSatisfactionThreshold);

	Genome* GetGenome(size_t aGenomeIdx) { return aGenomeIdx < myGenomes.size() ? &myGenomes[aGenomeIdx] : nullptr; }
	const Genome* GetBestGenome() const;

	std::vector<Specie*>& GetSpecies() { return mySpecies; }

	bool IsStagnant() const;

private:
	double GetAverageAdjustedFitness() const;

	void StartGeneration();
	void EndGeneration();

	std::vector<Genome> myGenomes;
	std::vector<Specie*> mySpecies;

	int myGeneration = -1;
	int myLastImprovementGeneration = -1;
	double myFitnessRecord = 0.0;
};

}
