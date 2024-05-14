#pragma once

#include <vector>

namespace Neat {

class Genome;

class Specie
{
public:
	size_t GetSize() const { return myGenomes.size(); }
	bool BelongsToSpecie(const Genome* aGenome) const;

	void AddGenome(Genome* aGenome) { myGenomes.push_back(aGenome); }
	void ClearGenomes() { myGenomes.clear(); }

	double GetBestFitness() const { return myBestFitness; }
	void ComputeBestFitness();
	void AdjustFitness();

	size_t ComputeOffspringsCount(double anAverageAdjustedFitness);
	void AllowExtraOffsprings(size_t anExtraOffspringsCount) { myOffspringsCount += anExtraOffspringsCount; }
	void ResetEvolution(size_t anOffspringsCount);

	void GenerateOffsprings();
	void CollectOffsprings(std::vector<Genome>& someOutOffsprings);

	bool IsNew() const;
	bool IsOld() const;
	bool IsStagnant() const;
	void ShouldExtinct() { myShouldExctinct = true; }
	void Age() { myAge++; }

private:
	std::vector<Genome*> myGenomes;
	double myBestFitness = 0.0;
	double myFitnessRecord = 0.0;

	int myAge = 0;
	int myLastImprovementAge = 0;
	bool myShouldExctinct = false;

	size_t myOffspringsCount = 0;
	std::vector<Genome> myOffsprings;
};

}
