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

	if (myGenomes.size() == 0)
	{
		if (someCallbacks.myOnTrainGenerationEnd)
			someCallbacks.myOnTrainGenerationEnd();
		return;
	}

	StartGeneration();

	if (someCallbacks.myEvaluateGenomes)
		someCallbacks.myEvaluateGenomes();

	for (Neat::Specie* specie : mySpecies)
		specie->ComputeBestFitness();

	std::sort(mySpecies.begin(), mySpecies.end(), [](const Specie* aSpecie1, const Specie* aSpecie2) { return aSpecie1->GetBestFitness() > aSpecie2->GetBestFitness(); });
	double bestFitness = mySpecies[0]->GetBestFitness();
	if (bestFitness > myFitnessRecord)
	{
		myFitnessRecord = bestFitness;
		myLastImprovementGeneration = myGeneration;
	}

	if (IsStagnant())
	{
		// If the population is stagnant as a whole, refocus on the 2 best species
		if (mySpecies.size() > 1)
		{
			size_t halfPopulationSize = myGenomes.size() / 2;
			mySpecies[0]->ResetEvolution(myGenomes.size() - halfPopulationSize);
			mySpecies[1]->ResetEvolution(halfPopulationSize);
			for (size_t i = 2; i < mySpecies.size(); ++i)
			{
				mySpecies[i]->ResetEvolution(0);
			}
		}
		else
		{
			mySpecies[0]->ResetEvolution(myGenomes.size());
		}
	}
	else
	{
		if (mySpecies.size() > 2)
		{
			// The least performing old specie should go extinct
			for (int i = (int)mySpecies.size() - 1; i >= 0; --i)
			{
				if (mySpecies[i]->IsOld())
				{
					mySpecies[i]->ShouldExtinct();
					break;
				}
			}
		}

		for (Neat::Specie* specie : mySpecies)
			specie->AdjustFitness();

		size_t offspringsCount = 0;
		double averageAdjustedFitness = GetAverageAdjustedFitness();
		for (Neat::Specie* specie : mySpecies)
			offspringsCount += specie->ComputeOffspringsCount(averageAdjustedFitness);
		if (myGenomes.size() > offspringsCount && mySpecies.size() > 0)
			mySpecies[0]->AllowExtraOffsprings(myGenomes.size() - offspringsCount);
	}

	// TODO : May want to parallelize this
	for (Neat::Specie* specie : mySpecies)
		specie->GenerateOffsprings();

	EndGeneration();

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

void Population::StartGeneration()
{
	myGeneration++;

	// First remove all the genomes from the species
	for (Specie* specie : mySpecies)
	{
		specie->ClearGenomes();
	}

	// And group all genomes (offsprings of the previous generation) that already know about their specie
	for (Genome& genome : myGenomes)
	{
		if (Specie* specie = genome.GetSpecie())
		{
			specie->AddGenome(&genome);
		}
	}

	// Then remove extinct species, age others
	for (auto it = mySpecies.begin(); it != mySpecies.end();)
	{
		if ((*it)->GetSize() == 0)
		{
			delete (*it);
			it = mySpecies.erase(it);
			continue;
		}
		else
		{
			(*it)->Age();
		}
		++it;
	}

	// Finally group the remaining genomes in their species, creating new species as necessary
	for (Genome& genome : myGenomes)
	{
		if (genome.GetSpecie())
			continue;

		if (genome.GetNodesCount() > 4)
			genome.SetFitness(genome.GetFitness());

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

void Population::EndGeneration()
{
	myGenomes.clear();
	for (Specie* specie : mySpecies)
	{
		specie->CollectOffsprings(myGenomes);
	}
}

}
