#include "Genome.h"
#include "EvolutionParams.h"
#include "Specie.h"
#include "Population.h"
#include <stdlib.h>
#include <random>

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

#define PI 3.14159265358979323846

struct PoleBalancingSystem
{
	PoleBalancingSystem(double aStartVariance);

	void Update(double aForce, double aDeltaTime);
	double GetPoleAngle() const { return std::atan2(std::sin(myPoleAngle), std::cos(myPoleAngle)); }
	bool IsPoleUp() const { return std::abs(GetPoleAngle()) <= myPoleFailureAngle; }
	void Draw();

	double myPoleAngle = 0.0;
	double myPoleVelocity = 0.0;
	double myPoleAcceleration = 0.0;
	double myPoleMass = 0.1;
	double myPoleLength = 0.5;

	double myCartPosition = 0.0;
	double myCartVelocity = 0.0;
	double myCartAcceleration = 0.0;
	double myCartMass = 1.0;

	double myGravitationalAcceleration = 9.81;
	double myCartForce = 5.0;
	double myTrackSize = 4.8;
	double myPoleFailureAngle = 0.209;
	double myPoleFriction = 0.1;
};

PoleBalancingSystem::PoleBalancingSystem(double aStartVariance)
{
	if (aStartVariance <= 0.0)
		return;
	std::normal_distribution<> rand(0.0, aStartVariance);
	myPoleAngle = rand(Neat::EvolutionParams::GetRandomGenerator()) * 2.0 * PI;
	myCartPosition = rand(Neat::EvolutionParams::GetRandomGenerator()) * myTrackSize / 2.0;
}

void PoleBalancingSystem::Update(double aForceAmplitude, double aDeltaTime)
{
	double sinAngle = std::sin(myPoleAngle);
	double cosAngle = std::cos(myPoleAngle);
	double force = aForceAmplitude * myCartForce;

	myPoleAcceleration = (myGravitationalAcceleration * sinAngle + cosAngle * ((-force - myPoleMass * myPoleLength * myPoleVelocity * myPoleVelocity * sinAngle) / (myCartMass + myPoleMass)))
		/ (myPoleLength * (4.0 / 3.0 - myPoleMass * cosAngle * cosAngle / (myCartMass + myPoleMass)));
	myPoleAcceleration -= myPoleVelocity * myPoleFriction;

	myCartAcceleration = (force + myPoleMass * myPoleLength * (myPoleVelocity * myPoleVelocity * sinAngle - myPoleAcceleration * cosAngle))
		/ (myCartMass + myPoleMass);

	myPoleVelocity += aDeltaTime * myPoleAcceleration;
	myPoleAngle += aDeltaTime * myPoleVelocity;

	myCartVelocity += aDeltaTime * myCartAcceleration;
	myCartPosition += aDeltaTime * myCartVelocity;

	if (std::abs(myCartPosition) > myTrackSize / 2.0)
	{
		myCartVelocity = 0.0;
		myCartPosition = (myCartPosition > 0.0 ? myTrackSize : -myTrackSize) / 2.0;
	}
}

void PoleBalancingSystem::Draw()
{
	static const double windowWidthPhysical = 10.0;

	ImVec2 size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	ImVec2 trackSize = ImVec2(size.x * static_cast<float>(myTrackSize / windowWidthPhysical), 20.f);
	ImVec2 trackPos = pos + ImVec2(size.x / 2.f, size.y / 2.f);

	ImVec2 cartSize = ImVec2(50.f, 30.f);
	ImVec2 cartPos = pos + ImVec2(size.x / 2.f + size.x * static_cast<float>(myCartPosition / windowWidthPhysical), size.y / 2.f);

	float poleSize = size.x * static_cast<float>(2.0 * myPoleLength / windowWidthPhysical);
	ImVec2 poleEndPos = cartPos + ImVec2(poleSize * static_cast<float>(std::sin(myPoleAngle)), -poleSize * static_cast<float>(std::cos(myPoleAngle)));

	ImU32 trackColor = IsPoleUp() ? 0xFF00FF00 : 0xFF0000FF;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(trackPos - trackSize / 2.f, trackPos + trackSize / 2.f, trackColor);
	draw_list->AddRectFilled(cartPos - cartSize / 2.f, cartPos + cartSize / 2.f, 0xFFFFFFFF);
	draw_list->AddLine(cartPos, poleEndPos, 0xFFFF0000, 5.f);

	double angleDegrees = GetPoleAngle() * 180.0 / PI;
	draw_list->AddText(cartPos + ImVec2(0.f, 50.f), 0xFFFFFFFF, std::format("Angle : {}", angleDegrees).c_str());
}

class NeatPoleBalancingModule : public Core::Module
{
	DECLARE_CORE_MODULE(NeatPoleBalancingModule, "NeatPoleBalancing")

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

	PoleBalancingSystem* mySystem = nullptr;
	bool myNeatControl = false;
	Neat::Genome* myNeatGenome = nullptr;
};

void NeatPoleBalancingModule::OnInitialize()
{
	Core::WindowModule::WindowParams params;
	params.myTitle = "NEAT - Pole Balancing";
	myWindow = Core::WindowModule::GetInstance()->OpenWindow(params);
	Render::RenderModule::GetInstance()->RegisterWindow(myWindow, Render::RendererType::GuiOnly);

	myGuiEntity = Core::Entity::Create();
	myGui = myGuiEntity.AddComponent<Render::EntityGuiComponent>(myWindow, false);
	myGui->myCallback = [this]() { OnGuiUpdate(); };

	mySystem = new PoleBalancingSystem(0.0);
	myNeatGenome = new Neat::Genome("poleBalancing");
}

void NeatPoleBalancingModule::OnFinalize()
{
	SafeDelete(myNeatGenome);
	SafeDelete(mySystem);

	myGuiEntity.Destroy();

	Render::RenderModule::GetInstance()->UnregisterWindow(myWindow);
	Core::WindowModule::GetInstance()->CloseWindow(myWindow);
}

void NeatPoleBalancingModule::OnUpdate(Core::Module::UpdateType aType)
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
			inputs.push_back(mySystem->myPoleAngle);
			inputs.push_back(mySystem->myPoleVelocity);
			inputs.push_back(mySystem->myCartPosition);
			inputs.push_back(mySystem->myCartVelocity);
			std::vector<double> outputs;
			myNeatGenome->Evaluate(inputs, outputs);

			mySystem->Update(outputs[0], Core::TimeModule::GetInstance()->GetDeltaTimeSec());
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

void NeatPoleBalancingModule::OnGuiUpdate()
{
	mySystem->Draw();
}

void EvaluatePopulationAsync(Thread::WorkerPool& aPool, Neat::Population& aPopulation, size_t aStartIdx, size_t aEndIdx)
{
	aPool.RequestJob([&aPopulation, aStartIdx, aEndIdx]() {
		for (size_t i = aStartIdx; i < aEndIdx; ++i)
		{
			//uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

			if (Neat::Genome* genome = aPopulation.GetGenome(i))
			{
				PoleBalancingSystem system(0.0);
				double fitness = 0.0;

				for (uint t = 0; t < 5000; ++t)
				{
					std::vector<double> inputs;
					inputs.push_back(system.myPoleAngle);
					inputs.push_back(system.myPoleVelocity);
					inputs.push_back(system.myCartPosition);
					inputs.push_back(system.myCartVelocity);
					std::vector<double> outputs;
					genome->Evaluate(inputs, outputs);

					system.Update(outputs[0], 0.02);

					if (system.IsPoleUp())
						fitness += 0.0002;
				}

				genome->SetFitness(fitness);
			}

			//uint64 duration = Core::TimeModule::GetInstance()->GetCurrentTimeMs() - startTime;
			//std::cout << i << " : " << duration << std::endl;
		}
	});
}

void TrainNeat()
{
	Thread::WorkerPool threadPool(Thread::WorkerPriority::High);
#if DEBUG_BUILD
	threadPool.SetWorkersCount(2); // Using several threads is slower in Debug...
#else
	threadPool.SetWorkersCount();
#endif

	std::random_device rd;
	Neat::EvolutionParams::SetRandomSeed(rd());
	//Neat::EvolutionParams::ourSpecieThreshold = 9999999.0; // TODO : remove, just for testing with 1 specie

	Neat::Population population = Neat::Population(100, 4, 1);
	Neat::Population::TrainingCallbacks callbacks;

	callbacks.myEvaluateGenomes = [&population, &threadPool]() {
		size_t runPerThread = population.GetSize() / threadPool.GetWorkersCount();
		size_t startIdx = 0;
		while (startIdx < population.GetSize())
		{
			EvaluatePopulationAsync(threadPool, population, startIdx, startIdx + runPerThread);
			startIdx = std::min(population.GetSize(), startIdx + runPerThread);
		}
		threadPool.WaitIdle();
	};

	callbacks.myGenerateOffsprings = [&population, &threadPool]() {
		for (Neat::Specie* specie : population.GetSpecies())
		{
			threadPool.RequestJob([specie]() {
				specie->GenerateOffsprings();
			});
		}
		threadPool.WaitIdle();
	};

	callbacks.myOnTrainGenerationEnd = [&population]() {
		std::cout << "Population Size : " << population.GetSize() << std::endl;
		std::cout << "Species Count : " << population.GetSpecies().size() << std::endl;
		if (const Neat::Genome* bestGenome = population.GetBestGenome())
		{
			std::cout << "Generation Best Fitness : " << bestGenome->GetFitness() << std::endl;
		}
	};

	uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

	population.TrainGenerations(callbacks, 100, 0.7);

	uint64 duration = Core::TimeModule::GetInstance()->GetCurrentTimeMs() - startTime;
	std::cout << "Training duration (ms) : " << duration << std::endl;

	if (const Neat::Genome* bestGenome = population.GetBestGenome())
	{
		bestGenome->SaveToFile("poleBalancing");
		std::cout << "Best Fitness : " << bestGenome->GetFitness() << std::endl;
	}
}

int main()
{
	InitMemoryLeaksDetection();

	Core::Facade::Create(__argc, __argv);

	TrainNeat();

	Render::RenderModule::Register();
	NeatPoleBalancingModule::Register();

	Core::Facade::GetInstance()->Run(NeatPoleBalancingModule::GetInstance()->GetWindow());

	NeatPoleBalancingModule::Unregister();
	Render::RenderModule::Unregister();

	Core::Facade::Destroy();

	return EXIT_SUCCESS;
}
