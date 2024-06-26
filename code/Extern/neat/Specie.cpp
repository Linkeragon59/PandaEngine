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
	
	size_t matchingGenesCount = 0;
	size_t nonMatchingGenesCount = 0;
	double averageWeightDifference = 0.0;

	for (auto it = representativeGenome->GetLinks().begin(); it != representativeGenome->GetLinks().end(); ++it)
	{
		std::uint64_t innovationId = it->first;
		const Link& representativeLink = it->second;

		auto it2 = aGenome->GetLinks().find(innovationId);
		if (it2 != aGenome->GetLinks().end())
		{
			// Common gene
			matchingGenesCount++;
			averageWeightDifference += std::abs(representativeLink.GetWeight() - it2->second.GetWeight());
		}
		else
		{
			// Disjoint or Excess gene
			nonMatchingGenesCount++;
		}
	}
	for (auto it = aGenome->GetLinks().begin(); it != aGenome->GetLinks().end(); ++it)
	{
		std::uint64_t innovationId = it->first;
		auto it2 = representativeGenome->GetLinks().find(innovationId);
		if (it2 == representativeGenome->GetLinks().end())
		{
			// Disjoint or Excess gene
			nonMatchingGenesCount++;
		}
	}

	if (matchingGenesCount > 0)
		averageWeightDifference /= matchingGenesCount;

	return (EvolutionParams::ourMatchingGeneCoeff * averageWeightDifference
		+ EvolutionParams::ourNonMatchingGeneCoeff * nonMatchingGenesCount) // TODO : Note the original paper divides by the genes count
		< EvolutionParams::ourSpecieThreshold;
}

void Specie::ComputeBestFitness()
{
	std::sort(myGenomes.begin(), myGenomes.end(), [](const Genome* aGenome1, const Genome* aGenome2) { return aGenome1->GetFitness() > aGenome2->GetFitness(); });
	if (myGenomes.size() > 0)
		myBestFitness = myGenomes[0]->GetFitness();

	if (myBestFitness > myFitnessRecord)
	{
		myFitnessRecord = myBestFitness;
		myLastImprovementAge = myAge;
	}
}

void Specie::AdjustFitness()
{
	for (Genome* genome : myGenomes)
	{
		double adjustedFitness = genome->GetFitness();

		if (myShouldExctinct || IsStagnant())
			adjustedFitness *= EvolutionParams::ourSpecieStagnantPenality;

		if (IsNew())
			adjustedFitness *= EvolutionParams::ourSpecieNewBonus;

		if (adjustedFitness < DBL_EPSILON)
			adjustedFitness = DBL_EPSILON;

		adjustedFitness /= myGenomes.size();

		genome->AdjustFitness(adjustedFitness);
	}
}

size_t Specie::ComputeOffspringsCount(double anAverageAdjustedFitness)
{
	double adjustedFitnessSum = 0.0;
	for (const Genome* genome : myGenomes)
		adjustedFitnessSum += genome->GetAdjustedFitness();
	myOffspringsCount = static_cast<size_t>(adjustedFitnessSum / anAverageAdjustedFitness);
	return myOffspringsCount;
}

void Specie::ResetEvolution(size_t anOffspringsCount)
{
	myOffspringsCount = anOffspringsCount;
	myLastImprovementAge = myAge;
}

void Specie::GenerateOffsprings()
{
	if (myOffspringsCount == 0)
		return;

	size_t countGenomesToMate = static_cast<size_t>(std::ceil(EvolutionParams::ourAmountGenomesToKeep * myGenomes.size()));
	myGenomes.resize(countGenomesToMate);

	if (countGenomesToMate == 0)
		return;

	// Copy the best genome as is for the next generation
	myOffsprings.push_back(Genome(*myGenomes[0]));
	myOffspringsCount--;

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

	while (myOffspringsCount > 0)
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
		myOffspringsCount--;
	}
}

void Specie::CollectOffsprings(std::vector<Genome>& someOutOffsprings)
{
	someOutOffsprings.reserve(someOutOffsprings.size() + myOffsprings.size());
	for (Genome& genome : myOffsprings)
		someOutOffsprings.push_back(std::move(genome));
	myOffsprings.clear();
}

bool Specie::IsNew() const
{
	return myAge <= EvolutionParams::ourSpecieNewThreshold;
}

bool Specie::IsOld() const
{
	return myAge > EvolutionParams::ourSpecieOldThreshold;
}

bool Specie::IsStagnant() const
{
	return myAge - myLastImprovementAge > EvolutionParams::ourSpecieStagnantThreshold;
}

}
