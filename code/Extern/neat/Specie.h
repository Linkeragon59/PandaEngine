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

	size_t ComputeNextSize(double anAverageAdjustedFitness);
	void AdjustNextSize(size_t aSize) { myNextSize = aSize; }
	void GenerateOffsprings();
	void CollectOffsprings(std::vector<Genome>& someOutOffsprings);

private:
	std::vector<Genome*> myGenomes;
	std::vector<Genome> myOffsprings;
	double myBestFitness = 0.0;
	int myNoImprovementCount = 0;
	size_t myNextSize = 0;
};

}
