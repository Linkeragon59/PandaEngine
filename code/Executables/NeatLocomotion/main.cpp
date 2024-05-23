#include "Character.h"

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

struct CharactersSystem
{
	CharactersSystem(const glm::vec2& aPlayerPosition, float aPlayerDirection, const glm::vec2& aNPCPosition, float aNPCDirection)
		: myPlayer(aPlayerPosition, aPlayerDirection, 20.f, glm::vec4(0.f, 0.f, 1.f, 1.f))
		, myNPC(aNPCPosition, aNPCDirection, 20.f, glm::vec4(1.f, 0.f, 0.f, 1.f))
		, myPlayerInitPos(aPlayerPosition)
		, myPlayerInitDir(aPlayerDirection)
		, myNPCInitPos(aNPCPosition)
		, myNPCInitDir(aNPCDirection)
	{}

	void Reset()
	{
		myPlayer.Reset(myPlayerInitPos, myPlayerInitDir);
		myNPC.Reset(myNPCInitPos, myNPCInitDir);
	}

	glm::vec2 myPlayerInitPos;
	glm::vec2 myNPCInitPos;
	float myPlayerInitDir;
	float myNPCInitDir;
	Character myPlayer;
	Character myNPC;
};
typedef std::vector<CharactersSystem> CharactersSystems;
typedef std::vector<CharactersSystems> CharactersSystemPool;

class NeatLocomotionModule : public Core::Module
{
	DECLARE_CORE_MODULE(NeatLocomotionModule, "NeatLocomotion")

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

	CharactersSystem* mySystem = nullptr;
	bool myNeatControl = false;
	Neat::Genome* myGenome = nullptr;
};

void NeatLocomotionModule::OnInitialize()
{
	Core::WindowModule::WindowParams params;
	params.myTitle = "NEAT - NPC Locomotion";
	myWindow = Core::WindowModule::GetInstance()->OpenWindow(params);
	Render::RenderModule::GetInstance()->RegisterWindow(myWindow, Render::RendererType::GuiOnly);

	myGuiEntity = Core::Entity::Create();
	myGui = myGuiEntity.AddComponent<Render::EntityGuiComponent>(myWindow, false);
	myGui->myCallback = [this]() { OnGuiUpdate(); };

	mySystem = new CharactersSystem(glm::vec2(-150.f, 0.f), (float)std::numbers::pi / 2.f, glm::vec2(150.f, 0.f), -(float)std::numbers::pi / 2.f);
	myGenome = new Neat::Genome("neat/locomotion");
}

void NeatLocomotionModule::OnFinalize()
{
	SafeDelete(myGenome);
	SafeDelete(mySystem);

	myGuiEntity.Destroy();

	Render::RenderModule::GetInstance()->UnregisterWindow(myWindow);
	Core::WindowModule::GetInstance()->CloseWindow(myWindow);
}

void NeatLocomotionModule::OnUpdate(Core::Module::UpdateType aType)
{
	if (aType == Core::Module::UpdateType::EarlyUpdate)
	{
		// Player Input
		{
			float forwardForce = 0.f;
			float rightForce = 0.f;
			float rotationForce = 0.f;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyNumPad8, myWindow) == Input::Status::Pressed)
				forwardForce += 1.f;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyNumPad2, myWindow) == Input::Status::Pressed)
				forwardForce -= 1.f;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyNumPad6, myWindow) == Input::Status::Pressed)
				rightForce += 1.f;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyNumPad4, myWindow) == Input::Status::Pressed)
				rightForce -= 1.f;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyNumPad9, myWindow) == Input::Status::Pressed)
				rotationForce += 1.f;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyNumPad7, myWindow) == Input::Status::Pressed)
				rotationForce -= 1.f;
			mySystem->myPlayer.Update(Core::TimeModule::GetInstance()->GetDeltaTimeSec(), forwardForce, rightForce, rotationForce);
		}

		// NPC locomotion

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
			float distanceInfo;
			float alignementInfo;
			float aimInfo;
			mySystem->myNPC.GetBrainInputs(mySystem->myPlayer, distanceInfo, alignementInfo, aimInfo);

			std::vector<double> inputs;
			inputs.push_back(distanceInfo);
			inputs.push_back(alignementInfo);
			inputs.push_back(aimInfo);
			std::vector<double> outputs;
			myGenome->Evaluate(inputs, outputs);

			float forwardForce = 0.f;
			float rightForce = 0.f;
			float rotationForce = 0.f;
			if (outputs[0] > outputs[1])
				forwardForce += 1.f;
			else
				forwardForce -= 1.f;
			if (outputs[2] > outputs[3])
				rightForce += 1.f;
			else
				rightForce -= 1.f;
			if (outputs[4] > outputs[5])
				rotationForce += 1.f;
			else
				rotationForce -= 1.f;
			mySystem->myNPC.Update(Core::TimeModule::GetInstance()->GetDeltaTimeSec(), forwardForce, rightForce, rotationForce);
		}
		else
		{
			mySystem->myNPC.Update(Core::TimeModule::GetInstance()->GetDeltaTimeSec(), 0.f, 0.f, 0.f);
		}
	}
	else if (aType == Core::Module::UpdateType::MainUpdate)
	{
		myGui->Update();
	}
}

void NeatLocomotionModule::OnGuiUpdate()
{
	mySystem->myPlayer.Draw();
	mySystem->myNPC.Draw();
	if (myNeatControl)
	{
		ImGui::Text("NEAT control");
	}
	ImGui::Text("Fitness: %.2f", mySystem->myNPC.ComputePositionFitness(mySystem->myPlayer));
}

void EvaluatePopulationAsync(Thread::WorkerPool& aPool, CharactersSystems& someSystems, Neat::Population& aPopulation, size_t aStartIdx, size_t aEndIdx)
{
	aPool.RequestJob([&someSystems, &aPopulation, aStartIdx, aEndIdx]() {
		for (size_t i = aStartIdx; i < aEndIdx; ++i)
		{
			if (Neat::Genome* genome = aPopulation.GetGenome(i))
			{
				double fitness = 0.0;

				float deltaTime = 0.02f;
				uint maxSteps = static_cast<uint>(20.f / deltaTime);
				double fitnessStep = 1.0 / static_cast<double>(maxSteps * someSystems.size());

				for (CharactersSystem system : someSystems)
				{
					system.Reset();

					for (uint t = 0; t < maxSteps; ++t)
					{
						float distanceInfo;
						float alignementInfo;
						float aimInfo;
						system.myNPC.GetBrainInputs(system.myPlayer, distanceInfo, alignementInfo, aimInfo);

						std::vector<double> inputs;
						inputs.push_back(distanceInfo);
						inputs.push_back(alignementInfo);
						inputs.push_back(aimInfo);
						std::vector<double> outputs;
						genome->Evaluate(inputs, outputs);

						float forwardForce = 0.f;
						float rightForce = 0.f;
						float rotationForce = 0.f;
						if (outputs[0] > outputs[1])
							forwardForce += 1.f;
						else
							forwardForce -= 1.f;
						if (outputs[2] > outputs[3])
							rightForce += 1.f;
						else
							rightForce -= 1.f;
						if (outputs[4] > outputs[5])
							rotationForce += 1.f;
						else
							rotationForce -= 1.f;
						system.myNPC.Update(deltaTime, forwardForce, rightForce, rotationForce);

						fitness += fitnessStep * system.myNPC.ComputePositionFitness(system.myPlayer);
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
	threadPool.SetWorkersCount();

	Neat::Population population = Neat::Population(300, 3, 6);
	Neat::Population::TrainingCallbacks callbacks;

	CharactersSystems systems;
	uint systemsCount = 100;
	systems.reserve(systemsCount);
	for (uint i = 0; i < systemsCount; ++i)
	{
		std::uniform_real_distribution<> randPos(-200.f, 200.f);
		std::uniform_real_distribution<> randAngle(-std::numbers::pi, std::numbers::pi);
	
		glm::vec2 playerPos = glm::vec2((float)randPos(Neat::EvolutionParams::GetRandomGenerator()), (float)randPos(Neat::EvolutionParams::GetRandomGenerator()));
		float playerDir = (float)randAngle(Neat::EvolutionParams::GetRandomGenerator());
		glm::vec2 npcPos = glm::vec2((float)randPos(Neat::EvolutionParams::GetRandomGenerator()), (float)randPos(Neat::EvolutionParams::GetRandomGenerator()));
		float npcDir = (float)randAngle(Neat::EvolutionParams::GetRandomGenerator());
	
		systems.push_back(CharactersSystem(playerPos, playerDir, npcPos, npcDir));
	}
	//systems.push_back(CharactersSystem(glm::vec2(-150.f, 0.f), (float)std::numbers::pi / 2.f, glm::vec2(150.f, 0.f), -(float)std::numbers::pi / 2.f));

	CharactersSystemPool systemsPool;
	systemsPool.resize(threadPool.GetWorkersCount(), systems);

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
		if (generationIdx % 10 == 0)
		{
			population.Check();
			std::cout << "Population Size : " << population.GetSize() << std::endl;
			std::cout << "Species Count : " << population.GetSpecies().size() << std::endl;
			if (const Neat::Genome* bestGenome = population.GetBestGenome())
			{
				std::cout << "Generation " << generationIdx << ": Best Fitness : " << bestGenome->GetFitness() << std::endl;

				std::string fileName = "neat/locomotion";
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
		bestGenome->SaveToFile("neat/locomotion");
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
	NeatLocomotionModule::Register();

	Core::Facade::GetInstance()->Run(NeatLocomotionModule::GetInstance()->GetWindow());

	NeatLocomotionModule::Unregister();
	Render::RenderModule::Unregister();

	Core::Facade::Destroy();

	return EXIT_SUCCESS;
}
