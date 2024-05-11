#include "Population.h"

namespace Neat {

Population::Population(size_t aCount, size_t anInputCount, size_t anOutputCount)
{
	Genome baseGenome = Genome(anInputCount, anOutputCount);

	myGenomes.reserve(aCount);
	for (size_t i = 0; i < aCount; ++i)
		myGenomes.push_back(baseGenome);
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

	GroupSpecies();

	if (someCallbacks.myEvaluateGenomes)
		someCallbacks.myEvaluateGenomes();

	double averageAdjustedFitness = GetAverageAdjustedFitness();
	for (Neat::Specie* specie : mySpecies)
	{
		specie->ComputeNextSize(averageAdjustedFitness);
	}

	// TODO : Adapt species sizes so the population count remains the same
	// Bug : Not sure why the best fitness can decrease even with no variance in the balancing system...
	// Problem : Mutations adding topology complexity are not protected at all? They should go into their own specie and be protected...

	if (someCallbacks.myGenerateOffsprings)
		someCallbacks.myGenerateOffsprings();

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
	double averageAdjustedFitness = 0.0;
	if (myGenomes.size() == 0)
		return averageAdjustedFitness;

	for (const Genome& genome : myGenomes)
		averageAdjustedFitness += genome.GetAdjustedFitness();
	return averageAdjustedFitness / myGenomes.size();
}

void Population::GroupSpecies()
{
	// First remove all the genomes from the species
	for (Specie* specie : mySpecies)
	{
		specie->ClearGenomes();
	}

	// And group all genomes (offsprings of the previous generation) who already know about their specie
	for (Genome& genome : myGenomes)
	{
		if (Specie* specie = genome.GetSpecie())
		{
			specie->AddGenome(&genome);
		}
	}

	// Then remove extinct species
	for (auto it = mySpecies.begin(); it != mySpecies.end();)
	{
		if ((*it)->GetSize() == 0)
		{
			delete (*it);
			it = mySpecies.erase(it);
			continue;
		}
		++it;
	}

	// Finally group the remaining genomes in their species, creating new species as necessary
	for (Genome& genome : myGenomes)
	{
		if (genome.GetSpecie())
			continue;

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
