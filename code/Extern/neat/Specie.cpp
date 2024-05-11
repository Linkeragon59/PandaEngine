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

	return (EvolutionParams::ourMatchingGeneCoeff * averageWeightDifference
		+ EvolutionParams::ourNonMatchingGeneCoeff * nonMatchingGenesCount / maxGenomeSize)
		< EvolutionParams::ourSpecieThreshold;
}

size_t Specie::ComputeNextSize(double anAverageAdjustedFitness)
{
	double newSize = 0.0;
	double bestFitness = 0.0;
	for (const Genome* genome : myGenomes)
	{
		newSize += genome->GetAdjustedFitness();
		if (genome->GetFitness() > bestFitness)
			bestFitness = genome->GetFitness();
	}

	if (bestFitness > myBestFitness)
		myNoImprovementCount = 0;
	else
		myNoImprovementCount++;
	myBestFitness = bestFitness;

	// If there was no improvement for many generations, don't generate any offsprings, and go extinct
	if (myNoImprovementCount > EvolutionParams::ourExtinctionAfterNoImprovement)
		return 0;
	
	myNextSize = static_cast<size_t>(std::round(newSize / anAverageAdjustedFitness));
	return myNextSize;
}

void Specie::GenerateOffsprings()
{
	if (myNextSize == 0)
		return;

	std::sort(myGenomes.begin(), myGenomes.end(), [](const Genome* aGenome1, const Genome* aGenome2) { return aGenome1->GetFitness() > aGenome2->GetFitness(); });
	size_t countGenomesToKeep = static_cast<size_t>(std::ceil(EvolutionParams::ourAmountGenomesToKeep * myGenomes.size()));
	myGenomes.resize(countGenomesToKeep);

	if (myGenomes.size() == 0)
		return;

	// Copy the best genome as is for the next generation
	myOffsprings.push_back(Genome(*myGenomes[0]));
	myNextSize--;

	// Generate as many offsprings as needed
	// randomly choose if we want only mutation or crossover
	// and select the parent(s) randomly, weighted by their fitness to favor performing genomes
	double totalFitness = 0.0;
	for (const Genome* genome : myGenomes)
		totalFitness += genome->GetFitness();
	std::uniform_real_distribution<> rand(0.0, totalFitness);
	auto getWeightedRandomGenome = [totalFitness, &rand, this]() -> const Genome* {
		double rng = rand(EvolutionParams::GetRandomGenerator());
		double cumulFitness = 0.0;
		for (const Genome* genome : myGenomes)
		{
			cumulFitness += genome->GetFitness();
			if (cumulFitness >= rng)
				return genome;
		}
		// Can't happen
		return myGenomes[0];
	};

	while (myNextSize > 0)
	{
		std::uniform_real_distribution<> rand2(0.0, 1.0);
		if (rand2(EvolutionParams::GetRandomGenerator()) < EvolutionParams::ourSingleParentReproductionProba)
		{
			// Offspring from one parent (mutation only)
			Genome& offspring = myOffsprings.emplace_back(*getWeightedRandomGenome());
			offspring.Mutate();
		}
		else
		{
			// Offspring from two parents + mutation
			Genome& offspring = myOffsprings.emplace_back(getWeightedRandomGenome(), getWeightedRandomGenome());
			offspring.Mutate();
		}
		myNextSize--;
	}
}

void Specie::CollectOffsprings(std::vector<Genome>& someOutOffsprings)
{
	someOutOffsprings.reserve(someOutOffsprings.size() + myOffsprings.size());
	for (Genome& genome : myOffsprings)
		someOutOffsprings.push_back(std::move(genome));
	myOffsprings.clear();
}

}
