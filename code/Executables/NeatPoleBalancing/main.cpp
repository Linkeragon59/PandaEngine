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
	PoleBalancingSystem(bool aRandomizeStart);
	void Reset();

	void Update(double aForce, double aDeltaTime);
	double GetPoleAngle() const { return std::atan2(std::sin(myPoleAngle), std::cos(myPoleAngle)); }
	bool IsPoleUp() const { return std::abs(GetPoleAngle()) <= myPoleFailureAngle; }
	void Draw();

	double myInitPoleAngle = PI;
	double myInitPoleVelocity = 0.0;
	double myInitCartPosition = 0.0;
	double myInitCartVelocity = 0.0;

	double myPoleAngle = 0.0;
	double myPoleVelocity = 0.0;
	double myPoleAcceleration = 0.0;
	double myPoleMass = 0.1;
	double myPoleLength = 0.5; // Half
	double myPoleFailureAngle = 0.2094384;
	double myPoleFriction = 0.1;

	double myCartPosition = 0.0;
	double myCartVelocity = 0.0;
	double myCartAcceleration = 0.0;
	double myCartMass = 1.0;
	double myCartTrackSize = 2.4; // Half
	double myCartFriction = 0.0;

	double myGravitationalAcceleration = 9.81;
	double myInputForce = 10.0;
};

typedef std::vector<PoleBalancingSystem> PoleBalancingSystems;
typedef std::vector<PoleBalancingSystems> PoleBalancingSystemPool;

PoleBalancingSystem::PoleBalancingSystem(bool aRandomizeStart)
{
	if (aRandomizeStart)
	{
		std::uniform_real_distribution<> rand(-1.0, 1.0);
		myInitPoleAngle = rand(Neat::EvolutionParams::GetRandomGenerator()) * PI;
		myInitPoleVelocity = rand(Neat::EvolutionParams::GetRandomGenerator()) * 3.0;
		myInitCartPosition = rand(Neat::EvolutionParams::GetRandomGenerator()) * myCartTrackSize;
		myInitCartVelocity = rand(Neat::EvolutionParams::GetRandomGenerator()) * 3.0;
	}
	Reset();
}

void PoleBalancingSystem::Reset()
{
	myPoleAngle = myInitPoleAngle;
	myPoleVelocity = myInitPoleVelocity;
	myCartPosition = myInitCartPosition;
	myCartVelocity = myInitCartVelocity;
}

void PoleBalancingSystem::Update(double aForceAmplitude, double aDeltaTime)
{
	double sinAngle = std::sin(myPoleAngle);
	double cosAngle = std::cos(myPoleAngle);
	double force = aForceAmplitude * myInputForce;

	if (force > 0.0 && myCartPosition >= myCartTrackSize)
		force = 0.0;
	else if (force < 0.0 && myCartPosition <= -myCartTrackSize)
		force = 0.0;

	double tmp = (force + myPoleMass * myPoleLength * myPoleVelocity * myPoleVelocity * sinAngle) / (myCartMass + myPoleMass);

	myPoleAcceleration = (myGravitationalAcceleration * sinAngle - cosAngle * tmp) / (myPoleLength * (4.0 / 3.0 - myPoleMass * cosAngle * cosAngle / (myCartMass + myPoleMass)));
	myPoleAcceleration -= myPoleVelocity * myPoleFriction;

	myCartAcceleration = tmp - myPoleMass * myPoleLength * myPoleVelocity * myPoleAcceleration * cosAngle / (myCartMass + myPoleMass);
	myCartAcceleration -= myCartVelocity * myCartFriction;
	
	myPoleVelocity += aDeltaTime * myPoleAcceleration;
	myPoleAngle += aDeltaTime * myPoleVelocity;

	myCartVelocity += aDeltaTime * myCartAcceleration;
	if (myCartVelocity > 0.0 && myCartPosition >= myCartTrackSize)
		myCartVelocity = 0.0;
	else if (myCartVelocity < 0.0 && myCartPosition <= -myCartTrackSize)
		myCartVelocity = 0.0;
	myCartPosition += aDeltaTime * myCartVelocity;
}

void PoleBalancingSystem::Draw()
{
	static const double windowWidthPhysical = 10.0;

	ImVec2 size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	ImVec2 trackSize = ImVec2(size.x * static_cast<float>(2.0 * myCartTrackSize / windowWidthPhysical), 20.f);
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
	draw_list->AddText(trackPos + ImVec2(0.f, 50.f), 0xFFFFFFFF, std::format("Angle : {}", angleDegrees).c_str());
	draw_list->AddText(trackPos + ImVec2(0.f, 75.f), 0xFFFFFFFF, std::format("Position : {}", myCartPosition).c_str());
	draw_list->AddText(trackPos + ImVec2(0.f, 100.f), 0xFFFFFFFF, std::format("Pole Velocity : {}", myPoleVelocity).c_str());
	draw_list->AddText(trackPos + ImVec2(0.f, 125.f), 0xFFFFFFFF, std::format("Cart Velocity : {}", myCartVelocity).c_str());
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

	mySystem = new PoleBalancingSystem(false);
	myNeatGenome = new Neat::Genome("poleBalancing320");
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

			double force = 1.0;
			if (outputs[0] < outputs[1])
				force = -1.0;
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

void NeatPoleBalancingModule::OnGuiUpdate()
{
	mySystem->Draw();
}

void EvaluatePopulationAsync(Thread::WorkerPool& aPool, PoleBalancingSystems& someSystems, Neat::Population& aPopulation, size_t aStartIdx, size_t aEndIdx)
{
	aPool.RequestJob([&someSystems, &aPopulation, aStartIdx, aEndIdx]() {
		for (size_t i = aStartIdx; i < aEndIdx; ++i)
		{
			if (Neat::Genome* genome = aPopulation.GetGenome(i))
			{
				double fitness = 0.0;
				
				double deltaTime = 0.02;
				uint maxSteps = static_cast<uint>(10.0 / deltaTime);
				double fitnessStep = 1.0 / static_cast<double>(maxSteps * someSystems.size());

				for (PoleBalancingSystem system : someSystems)
				{
					system.Reset();

					for (uint t = 0; t < maxSteps; ++t)
					{
						std::vector<double> inputs;
						inputs.push_back(system.myPoleAngle);
						inputs.push_back(system.myPoleVelocity);
						inputs.push_back(system.myCartPosition);
						inputs.push_back(system.myCartVelocity);
						std::vector<double> outputs;
						genome->Evaluate(inputs, outputs);

						double force = 1.0;
						if (outputs[0] < outputs[1])
							force = -1.0;
						system.Update(force, deltaTime);
						if (!system.IsPoleUp())
							continue;

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

	PoleBalancingSystems systems;
	uint systemsCount = 50;
	systems.reserve(systemsCount);
	for (uint i = 0; i < systemsCount; ++i)
		systems.push_back(true);

	PoleBalancingSystemPool systemsPool;
	systemsPool.resize(threadPool.GetWorkersCount(), systems);

	Neat::Population population = Neat::Population(200, 4, 2);
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
				std::string fileName = "poleBalancing";
				fileName += std::format("{}", generationIdx);
				bestGenome->SaveToFile(fileName.c_str());
			}
		}
		generationIdx++;
	};

	uint64 startTime = Core::TimeModule::GetInstance()->GetCurrentTimeMs();

	population.TrainGenerations(callbacks, 1000, 1.0);

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

	std::random_device rd;
	unsigned int seed = rd();
	Neat::EvolutionParams::SetRandomSeed(seed);

	TrainNeat();

	Render::RenderModule::Register();
	NeatPoleBalancingModule::Register();

	Core::Facade::GetInstance()->Run(NeatPoleBalancingModule::GetInstance()->GetWindow());

	NeatPoleBalancingModule::Unregister();
	Render::RenderModule::Unregister();

	Core::Facade::Destroy();

	return EXIT_SUCCESS;
}
