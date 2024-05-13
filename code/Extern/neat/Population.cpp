#include "Population.h"
#include <algorithm>

namespace Neat {

Population::Population(size_t aCount, size_t anInputCount, size_t anOutputCount)
{
	Genome baseGenome = Genome(anInputCount, anOutputCount);

	myGenomes.reserve(aCount);
	for (size_t i = 0; i < aCount; ++i)
	{
		Genome& genome = myGenomes.emplace_back(baseGenome);
		genome.Mutate();
	}

	GroupSpecies();
}

Population::~Population()
{
	for (Specie* specie : mySpecies)
		delete specie;
}

void Population::TrainOneGeneration(const TrainingCallbacks& someCallbacks)
{
	if (someCallbacks.myOnTrainGenerationStart)
		someCallbacks.myOnTrainGenerationStart();

	if (someCallbacks.myEvaluateGenomes)
		someCallbacks.myEvaluateGenomes();

	for (Neat::Specie* specie : mySpecies)
		specie->ComputeBestFitness();

	std::sort(mySpecies.begin(), mySpecies.end(), [](const Specie* aSpecie1, const Specie* aSpecie2) { return aSpecie1->GetBestFitness() > aSpecie2->GetBestFitness(); });

	// The least performing old specie will go extinct
	for (size_t i = mySpecies.size() - 1; i >= 0; --i)
	{
		if (mySpecies[i]->IsOld())
		{
			mySpecies[i]->GoExtinct();
			break;
		}
	}

	for (Neat::Specie* specie : mySpecies)
	{
		specie->AdjustFitness();
	}

	// TODO : Adapt species sizes so the population count remains the same
	// Bug : Not sure why the best fitness can decrease even with no variance in the balancing system...
	// Problem : Mutations adding topology complexity are not protected at all? They should go into their own specie and be protected...
	for (Neat::Specie* specie : mySpecies)
	{
		specie->GenerateOffsprings();
	}

	ReplacePopulationWithOffsprings();

	if (someCallbacks.myOnTrainGenerationEnd)
		someCallbacks.myOnTrainGenerationEnd();
}

void Population::TrainGenerations(const TrainingCallbacks& someCallbacks, int aMaxGenerationCount, double aSatisfactionThreshold)
{
	for (int i = 0; i < aMaxGenerationCount; ++i)
	{
		TrainOneGeneration(someCallbacks);
		const Genome* bestGenome = GetBestGenome();
		if (bestGenome && bestGenome->GetFitness() > aSatisfactionThreshold)
			break;
	}
}

const Genome* Population::GetBestGenome() const
{
	const Genome* bestGenome = nullptr;
	double bestFitness = -DBL_MAX;
	for (const Genome& genome : myGenomes)
	{
		if (genome.GetFitness() > bestFitness)
		{
			bestGenome = &genome;
			bestFitness = genome.GetFitness();
		}
	}
	return bestGenome;
}

double Population::GetAverageAdjustedFitness() const
{
	if (myGenomes.size() == 0)
		return 0.0;

	double averageAdjustedFitness = 0.0;
	for (const Genome& genome : myGenomes)
		averageAdjustedFitness += genome.GetAdjustedFitness();
	return averageAdjustedFitness / myGenomes.size();
}

void Population::GroupSpecies()
{
	mySpecies.clear();

	for (Genome& genome : myGenomes)
	{
		genome.SetSpecie(nullptr);

		for (Specie* specie : mySpecies)
		{
			if (specie->BelongsToSpecie(&genome))
			{
				genome.SetSpecie(specie);
				specie->AddGenome(&genome);
				break;
			}
		}

		if (!genome.GetSpecie())
		{
			Specie* newSpecie = new Specie;
			genome.SetSpecie(newSpecie);
			newSpecie->AddGenome(&genome);
			mySpecies.push_back(newSpecie);
		}
	}
}

void Population::ReplacePopulationWithOffsprings()
{
	myGenomes.clear();
	for (Specie* specie : mySpecies)
	{
		specie->CollectOffsprings(myGenomes);
	}
}

}
