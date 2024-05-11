#include "Specie.h"

#include "EvolutionParams.h"
#include "Genome.h"

namespace Neat {

bool Specie::BelongsToSpecie(const Genome* aGenome) const
{
	if (myGenomes.empty())
		return false;

	std::uniform_int_distribution<> rand(0, (int)myGenomes.size() - 1);
	const Genome* representativeGenome = myGenomes[rand(EvolutionParams::GetRandomGenerator())];

	size_t maxGenomeSize = std::max(representativeGenome->GetEdges().size(), aGenome->GetEdges().size());
	if (maxGenomeSize == 0)
		return false;
	
	size_t matchingGenesCount = 0;
	size_t nonMatchingGenesCount = 0;
	double averageWeightDifference = 0.0;

	for (auto it = representativeGenome->GetEdges().begin(); it != representativeGenome->GetEdges().end(); ++it)
	{
		std::uint64_t innovationId = it->first;
		const Edge& representativeEdge = it->second;

		auto it2 = aGenome->GetEdges().find(innovationId);
		if (it2 != aGenome->GetEdges().end())
		{
			// Common gene
			matchingGenesCount++;
			averageWeightDifference += std::abs(representativeEdge.GetWeight() - it2->second.GetWeight());
		}
		else
		{
			// Disjoint or Excess gene
			nonMatchingGenesCount++;
		}
	}

	if (matchingGenesCount > 0)
		averageWeightDifference /= matchingGenesCount;

	// TODO : weird choice of params, can't really fail this test...
	return (EvolutionParams::ourMatchingGeneCoeff * averageWeightDifference
		+ EvolutionParams::ourNonMatchingGeneCoeff * nonMatchingGenesCount / maxGenomeSize)
		< EvolutionParams::ourSpecieThreshold;
}

size_t Specie::ComputeNextSize(double anAverageAdjustedFitness)
{
	myNoImprovementCount++;

	double newSize = 0.0;
	for (const Genome* genome : myGenomes)
	{
		newSize += genome->GetAdjustedFitness();
		if (genome->GetFitness() > myBestFitness)
		{
			myBestFitness = genome->GetFitness();
			myNoImprovementCount = 0;
		}
	}

	// If there was no improvement for many generations, don't generate any offsprings, and go extinct
	if (myNoImprovementCount > EvolutionParams::ourExtinctionAfterNoImprovement)
		myNextSize = 0;
	else
		myNextSize = static_cast<size_t>(std::round(newSize / anAverageAdjustedFitness));

	return myNextSize;
}

void Specie::GenerateOffsprings()
{
	if (myNextSize == 0)
		return;

	// TODO
	// Sort the genomes by fitness,
	// put a copy of the champion in the offsprings
	// while we can still generate offsprings, randomly choose if we want only mutation or crossover, and select the parents randomely, weighted by their fitness
}

void Specie::CollectOffsprings(std::vector<Genome>& someOutOffsprings)
{
	someOutOffsprings.reserve(someOutOffsprings.size() + myOffsprings.size());
	for (Genome& genome : myOffsprings)
		someOutOffsprings.push_back(std::move(genome));
	myOffsprings.clear();
}

}
