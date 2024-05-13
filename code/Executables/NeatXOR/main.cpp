#include "Genome.h"
#include "EvolutionParams.h"
#include "Specie.h"
#include "Population.h"
#include <stdlib.h>
#include <random>

#include <iostream>

#include "Core_Thread.h"
#include "Core_TimeModule.h"

void EvaluatePopulationAsync(Thread::WorkerPool& aPool, Neat::Population& aPopulation, size_t aStartIdx, size_t aEndIdx)
{
	aPool.RequestJob([&aPopulation, aStartIdx, aEndIdx]() {
		for (size_t i = aStartIdx; i < aEndIdx; ++i)
		{
			if (Neat::Genome* genome = aPopulation.GetGenome(i))
			{
				double error = 0.0;

				double xorInputs[4][2] = {
					{0.0, 0.0},
					{0.0, 1.0},
					{1.0, 0.0},
					{1.0, 1.0}
				};
				double xorOutputs[4] = {
					0.0,
					1.0,
					1.0,
					0.0
				};

				for (uint j = 0; j < 4; ++j)
				{
					std::vector<double> inputs;
					inputs.push_back(xorInputs[j][0]);
					inputs.push_back(xorInputs[j][1]);
					std::vector<double> outputs;
					genome->Evaluate(inputs, outputs);

					error += std::abs(xorOutputs[j] - outputs[0]);
				}

				genome->SetFitness(std::pow(4.0 - error, 2.0)); // [0, 16]
			}
		}
	});
}

void TrainNeat()
{
	Thread::WorkerPool threadPool(Thread::WorkerPriority::High);
#if DEBUG_BUILD
	threadPool.SetWorkersCount(1); // Using several threads is slower in Debug...
#else
	threadPool.SetWorkersCount();
#endif

	std::random_device rd;
	Neat::EvolutionParams::SetRandomSeed(rd());

	Neat::Population population = Neat::Population(150, 2, 1);
	Neat::Population::TrainingCallbacks callbacks;

	callbacks.myEvaluateGenomes = [&population, &threadPool]() {
		size_t runPerThread = population.GetSize() / threadPool.GetWorkersCount() + 1;
		size_t startIdx = 0;
		while (startIdx < population.GetSize())
		{
			EvaluatePopulationAsync(threadPool, population, startIdx, startIdx + runPerThread);
			startIdx = std::min(population.GetSize(), startIdx + runPerThread);
		}
		threadPool.WaitIdle();
	};

	int generationIdx = 0;
	callbacks.myOnTrainGenerationEnd = [&population, &generationIdx]() {
		std::cout << "Population Size : " << population.GetSize() << std::endl;
		std::cout << "Species Count : " << population.GetSpecies().size() << std::endl;
		if (const Neat::Genome* bestGenome = population.GetBestGenome())
		{
			std::cout << "Generation " << generationIdx << ": Best Fitness : " << bestGenome->GetFitness() << std::endl;
		}
		generationIdx++;
	};

	uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

	population.TrainGenerations(callbacks, 1000, 14.0);

	uint64 duration = Core::TimeModule::GetInstance()->GetCurrentTimeMs() - startTime;
	std::cout << "Training duration (ms) : " << duration << std::endl;

	if (const Neat::Genome* bestGenome = population.GetBestGenome())
	{
		bestGenome->SaveToFile("neat_xor");
		std::cout << "Best Fitness : " << bestGenome->GetFitness() << std::endl;
	}
}

int main()
{
	InitMemoryLeaksDetection();

	Core::Facade::Create(__argc, __argv);

	TrainNeat();

	Neat::Genome genome("neat_xor");
	{
		double error = 0.0;

		double xorInputs[4][2] = {
			{0.0, 0.0},
			{0.0, 1.0},
			{1.0, 0.0},
			{1.0, 1.0}
		};
		double xorOutputs[4] = {
			0.0,
			1.0,
			1.0,
			0.0
		};

		for (uint j = 0; j < 4; ++j)
		{
			std::vector<double> inputs;
			inputs.push_back(xorInputs[j][0]);
			inputs.push_back(xorInputs[j][1]);
			std::vector<double> outputs;
			genome.Evaluate(inputs, outputs);
			std::cout << "XOR(" << xorInputs[j][0] << "," << xorInputs[j][1] << ") = " << outputs[0] << " (error: " << std::abs(xorOutputs[j] - outputs[0]) << ")" << std::endl;
		}

		genome.SetFitness(std::pow(4.0 - error, 2.0)); // [0, 16]}
	}

	Core::Facade::Destroy();

	return EXIT_SUCCESS;
}
