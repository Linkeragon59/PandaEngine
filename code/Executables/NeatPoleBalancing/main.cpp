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

	draw_list->AddText(cartPos + ImVec2(0.f, 50.f), 0xFFFFFFFF, std::format("Angle : {}", GetPoleAngle() * 180.0 / PI).c_str());
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

	PoleBalancingSystem mySystem;
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

	myNeatGenome = new Neat::Genome("poleBalancing");
}

void NeatPoleBalancingModule::OnFinalize()
{
	SafeDelete(myNeatGenome);

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
			inputs.push_back(mySystem.myPoleAngle);
			inputs.push_back(mySystem.myPoleVelocity);
			inputs.push_back(mySystem.myCartPosition);
			inputs.push_back(mySystem.myCartVelocity);
			std::vector<double> outputs;
			myNeatGenome->Evaluate(inputs, outputs);

			mySystem.Update(outputs[0], Core::TimeModule::GetInstance()->GetDeltaTimeSec());
		}
		else
		{
			double aForceAmplitude = 0.0;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyLeft, myWindow) == Input::Status::Pressed)
				aForceAmplitude -= 1.0;
			if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyRight, myWindow) == Input::Status::Pressed)
				aForceAmplitude += 1.0;

			mySystem.Update(aForceAmplitude, Core::TimeModule::GetInstance()->GetDeltaTimeSec());
		}
	}
	else if (aType == Core::Module::UpdateType::MainUpdate)
	{
		myGui->Update();
	}
}

void NeatPoleBalancingModule::OnGuiUpdate()
{
	mySystem.Draw();
}

void EvaluatePopulationAsync(Thread::WorkerPool& aPool, Neat::Population& aPopulation, size_t aStartIdx, size_t aEndIdx)
{
	aPool.RequestJob([&aPopulation, aStartIdx, aEndIdx]() {
		for (size_t i = aStartIdx; i < aEndIdx; ++i)
		{
			uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

			if (Neat::Genome* genome = aPopulation.GetGenome(i))
			{
				PoleBalancingSystem system;
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

			uint64 duration = Core::TimeModule::GetInstance()->GetCurrentTimeMs() - startTime;

			std::cout << i << " : " << duration << std::endl;
		}
	});
}

void TrainNeatOneGeneration(Thread::WorkerPool& aPool, Neat::Population& aPopulation)
{
	uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

	aPopulation.GroupSpecies();

	size_t runPerThread = aPopulation.GetSize() / aPool.GetWorkersCount();
	size_t startIdx = 0;
	while (startIdx < aPopulation.GetSize())
	{
		EvaluatePopulationAsync(aPool, aPopulation, startIdx, startIdx + runPerThread);
		startIdx = std::min(aPopulation.GetSize(), startIdx + runPerThread);
	}

	aPool.WaitIdle();

	for (Neat::Specie& specie : aPopulation.GetSpecies())
	{
		aPool.RequestJob([&specie]() {
			specie.GenerateOffsprings();
		});
	}

	aPool.WaitIdle();

	aPopulation.ReplacePopulationWithOffsprings();

	uint64 duration = Core::TimeModule::GetInstance()->GetCurrentTimeMs() - startTime;
	std::cout << duration << std::endl;
}

void TrainNeat()
{
	std::random_device rd;
	Neat::EvolutionParams::SetRandomSeed(rd());

	Thread::WorkerPool threadPool(Thread::WorkerPriority::High);
#if DEBUG_BUILD
	threadPool.SetWorkersName("Workers");
	threadPool.SetWorkersCount(1); // Using several threads is slower in Debug...
#else
	threadPool.SetWorkersCount();
#endif

	Neat::Population population = Neat::Population(100, 4, 1);
	const Neat::Genome* bestGenome = nullptr;

	double fitnessThreshold = 0.5;
	for (uint i = 0; i < 1; ++i)
	{
		TrainNeatOneGeneration(threadPool, population);
		bestGenome = population.GetBestGenome();
		if (bestGenome && bestGenome->GetFitness() > fitnessThreshold)
			break;
	}

	if (bestGenome)
		bestGenome->SaveToFile("poleBalancing");
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
