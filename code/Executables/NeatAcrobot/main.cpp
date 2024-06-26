#include "Acrobot.h"

#include "Genome.h"
#include "EvolutionParams.h"
#include "Specie.h"
#include "Population.h"

#include "Core_Facade.h"
#include "Core_Module.h"
#include "Core_WindowModule.h"
#include "Core_TimeModule.h"
#include "Core_InputModule.h"
#include "Render_RenderModule.h"
#include "Core_Thread.h"
#include "Core_Entity.h"
#include "Render_EntityRenderComponent.h"
#include "imgui_helpers.h"

#include <iostream>
#include <random>

class NeatAcrobotModule : public Core::Module
{
	DECLARE_CORE_MODULE(NeatAcrobotModule, "NeatAcrobot")

public:
	GLFWwindow* GetWindow() const { return myWindow; }

protected:
	void OnInitialize() override;
	void OnFinalize() override;
	void OnUpdate(Core::Module::UpdateType aType) override;

private:
	void OnGuiUpdate();

	GLFWwindow* myWindow = nullptr;
	Core::Entity myGuiEntity;
	Render::EntityGuiComponent* myGui = nullptr;

	Acrobot* mySystem = nullptr;
	bool myNeatControl = false;
	Neat::Genome* myBalancingGenome = nullptr;
};

void NeatAcrobotModule::OnInitialize()
{
	Core::WindowModule::WindowParams params;
	params.myTitle = "NEAT - Acrobot";
	myWindow = Core::WindowModule::GetInstance()->OpenWindow(params);
	Render::RenderModule::GetInstance()->RegisterWindow(myWindow, Render::RendererType::GuiOnly);

	myGuiEntity = Core::Entity::Create();
	myGui = myGuiEntity.AddComponent<Render::EntityGuiComponent>(myWindow, false);
	myGui->myCallback = [this]() { OnGuiUpdate(); };

	mySystem = new Acrobot(false, 0.0);
	myBalancingGenome = new Neat::Genome("neat/acrobot");
}

void NeatAcrobotModule::OnFinalize()
{
	SafeDelete(myBalancingGenome);
	SafeDelete(mySystem);

	myGuiEntity.Destroy();

	Render::RenderModule::GetInstance()->UnregisterWindow(myWindow);
	Core::WindowModule::GetInstance()->CloseWindow(myWindow);
}

void NeatAcrobotModule::OnUpdate(Core::Module::UpdateType aType)
{
	if (aType == Core::Module::UpdateType::EarlyUpdate)
	{
		static bool enterPressed = false;
		if (!enterPressed && Core::InputModule::GetInstance()->PollKeyInput(Input::KeyEnter, myWindow) == Input::Status::Pressed)
		{
			enterPressed = true;
			myNeatControl = !myNeatControl;
		}
		if (enterPressed && Core::InputModule::GetInstance()->PollKeyInput(Input::KeyEnter, myWindow) == Input::Status::Released)
		{
			enterPressed = false;
		}

		if (myNeatControl)
		{
			std::vector<double> inputs;
			inputs.push_back(mySystem->GetPole1Angle());
			inputs.push_back(mySystem->GetPole2Angle());
			inputs.push_back(mySystem->GetPole1Velocity());
			inputs.push_back(mySystem->GetPole2Velocity());
			std::vector<double> outputs;
			myBalancingGenome->Evaluate(inputs, outputs);

			double force = 0.0;
			if (outputs[0] > outputs[1] && outputs[0] > outputs[2])
				force = -1.0;
			else if (outputs[2] > outputs[0] && outputs[2] > outputs[1])
				force = +1.0;

			mySystem->Update(force, Core::TimeModule::GetInstance()->GetDeltaTimeSec());
		}
		else
		{
			double aForceAmplitude = 0.0;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyLeft, myWindow) == Input::Status::Pressed)
				aForceAmplitude -= 1.0;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyRight, myWindow) == Input::Status::Pressed)
				aForceAmplitude += 1.0;

			mySystem->Update(aForceAmplitude, Core::TimeModule::GetInstance()->GetDeltaTimeSec());
		}
	}
	else if (aType == Core::Module::UpdateType::MainUpdate)
	{
		myGui->Update();
	}
}

void NeatAcrobotModule::OnGuiUpdate()
{
	mySystem->Draw();
	if (myNeatControl)
	{
		ImGui::Text("NEAT control");
	}
	else
	{
		ImGui::Text("Manual control");
	}
}

void EvaluatePopulationAsync(Thread::WorkerPool& aPool, Acrobots& someSystems, Neat::Population& aPopulation, size_t aStartIdx, size_t aEndIdx)
{
	aPool.RequestJob([&someSystems, &aPopulation, aStartIdx, aEndIdx]() {
		for (size_t i = aStartIdx; i < aEndIdx; ++i)
		{
			if (Neat::Genome* genome = aPopulation.GetGenome(i))
			{
				double fitness = 0.0;

				double deltaTime = 0.02;
				uint maxSteps = static_cast<uint>(25.0 / deltaTime);
				double fitnessStep = 1.0 / static_cast<double>(maxSteps * someSystems.size());

				for (Acrobot system : someSystems)
				{
					system.Reset();

					for (uint t = 0; t < maxSteps; ++t)
					{
						std::vector<double> inputs;
						inputs.push_back(system.GetPole1Angle());
						inputs.push_back(system.GetPole2Angle());
						inputs.push_back(system.GetPole1Velocity());
						inputs.push_back(system.GetPole2Velocity());
						std::vector<double> outputs;
						genome->Evaluate(inputs, outputs);

						double force = 0.0;
						if (outputs[0] > outputs[1] && outputs[0] > outputs[2])
							force = -1.0;
						else if (outputs[2] > outputs[0] && outputs[2] > outputs[1])
							force = +1.0;

						system.Update(force, deltaTime);

						if (system.ArePolesUp())
							fitness += fitnessStep;
					}
				}

				genome->SetFitness(fitness);
			}
		}
	});
}

void TrainNeat()
{
	Thread::WorkerPool threadPool(Thread::WorkerPriority::High);
#if DEBUG_BUILD
	threadPool.SetWorkersCount(3); // Using several threads is slower in Debug...
#else
	threadPool.SetWorkersCount();
#endif

	Acrobots systems;
	uint systemsCount = 4;
	systems.reserve(2 * (systemsCount + 1));
	//systems.push_back(Acrobot(true, 0.0));
	//for (uint i = 0; i < systemsCount; ++i)
	//	systems.push_back(Acrobot(true, 0.1));
	systems.push_back(Acrobot(false, 0.0));
	//for (uint i = 0; i < systemsCount; ++i)
	//	systems.push_back(Acrobot(false, 0.1));

	AcrobotPool systemsPool;
	systemsPool.resize(threadPool.GetWorkersCount(), systems);

	Neat::Population population = Neat::Population(500, 4, 3);
	Neat::Population::TrainingCallbacks callbacks;

	callbacks.myEvaluateGenomes = [&population, &threadPool, &systemsPool]() {
		size_t runPerThread = population.GetSize() / threadPool.GetWorkersCount() + 1;
		size_t startIdx = 0;
		uint systemPoolIdx = 0;
		while (startIdx < population.GetSize())
		{
			EvaluatePopulationAsync(threadPool, systemsPool[systemPoolIdx], population, startIdx, startIdx + runPerThread);
			startIdx = std::min(population.GetSize(), startIdx + runPerThread);
			systemPoolIdx++;
		}
		threadPool.WaitIdle();
		};

	int generationIdx = 0;
	callbacks.myOnTrainGenerationEnd = [&population, &generationIdx]() {
		population.Check();
		std::cout << "Population Size : " << population.GetSize() << std::endl;
		std::cout << "Species Count : " << population.GetSpecies().size() << std::endl;
		if (const Neat::Genome* bestGenome = population.GetBestGenome())
		{
			std::cout << "Generation " << generationIdx << ": Best Fitness : " << bestGenome->GetFitness() << std::endl;

			if (generationIdx % 100 == 0)
			{
				std::string fileName = "neat/acrobot";
				fileName += std::format("_{}", generationIdx);
				bestGenome->SaveToFile(fileName.c_str());
			}
		}
		generationIdx++;
		};

	uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

	population.TrainGenerations(callbacks, 1000, DBL_MAX);

	uint64 duration = Core::TimeModule::GetInstance()->GetCurrentTimeMs() - startTime;
	std::cout << "Training duration (ms) : " << duration << std::endl;

	if (const Neat::Genome* bestGenome = population.GetBestGenome())
	{
		bestGenome->SaveToFile("neat/acrobot");
		std::cout << "Best Fitness : " << bestGenome->GetFitness() << std::endl;
	}
}

int main()
{
	InitMemoryLeaksDetection();

	Core::Facade::Create(__argc, __argv);

	std::random_device rd;
	unsigned int seed = rd();
	Neat::EvolutionParams::SetRandomSeed(seed);

	TrainNeat();

	Render::RenderModule::Register();
	NeatAcrobotModule::Register();

	Core::Facade::GetInstance()->Run(NeatAcrobotModule::GetInstance()->GetWindow());

	NeatAcrobotModule::Unregister();
	Render::RenderModule::Unregister();

	Core::Facade::Destroy();

	return EXIT_SUCCESS;
}
