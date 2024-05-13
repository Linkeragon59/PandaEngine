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

	void ComputeBestFitness();
	void PostEvaluation();
	void AdjustFitness();

	double GetBestFitness() const { return myBestFitness; }
	void AdjustNextSize(size_t aSize) { myNextSize = aSize; }

	void GenerateOffsprings();
	void CollectOffsprings(std::vector<Genome>& someOutOffsprings);

	bool IsNew() const { return myAge <= 100; } // TODO : Should be a parameter
	bool IsOld() const { return myAge > 20; } // TODO : Should be a parameter
	bool IsStagnant() const { return myLastImprovementAge > myAge + 15; } // TODO : Should be a parameter
	void GoExtinct() { myGoExctinct = true; }

private:
	std::vector<Genome*> myGenomes;
	double myBestFitness = 0.0;
	double myFitnessRecord = 0.0;

	std::vector<Genome> myOffsprings;

	int myNoImprovementCount = 0;
	int myAge = 0;
	int myLastImprovementAge = 0;
	bool myGoExctinct = false;
	size_t myNextSize = 0;
};

}
