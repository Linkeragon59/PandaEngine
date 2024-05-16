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

#include "CartPole.h"

#include <iostream>

class NeatDoublePoleBalancingModule : public Core::Module
{
	DECLARE_CORE_MODULE(NeatDoublePoleBalancingModule, "NeatDoublePoleBalancing")

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

	CartDoublePole* mySystem = nullptr;
};

void NeatDoublePoleBalancingModule::OnInitialize()
{
	Core::WindowModule::WindowParams params;
	params.myTitle = "NEAT - Pole Balancing";
	myWindow = Core::WindowModule::GetInstance()->OpenWindow(params);
	Render::RenderModule::GetInstance()->RegisterWindow(myWindow, Render::RendererType::GuiOnly);

	myGuiEntity = Core::Entity::Create();
	myGui = myGuiEntity.AddComponent<Render::EntityGuiComponent>(myWindow, false);
	myGui->myCallback = [this]() { OnGuiUpdate(); };

	mySystem = new CartDoublePole();
}

void NeatDoublePoleBalancingModule::OnFinalize()
{
	SafeDelete(mySystem);

	myGuiEntity.Destroy();

	Render::RenderModule::GetInstance()->UnregisterWindow(myWindow);
	Core::WindowModule::GetInstance()->CloseWindow(myWindow);
}

void NeatDoublePoleBalancingModule::OnUpdate(Core::Module::UpdateType aType)
{
	if (aType == Core::Module::UpdateType::EarlyUpdate)
	{
		double aForceAmplitude = 0.0;
		if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyLeft, myWindow) == Input::Status::Pressed)
			aForceAmplitude -= 1.0;
		if (Core::InputModule::GetInstance()->PollKeyInput(Input::KeyRight, myWindow) == Input::Status::Pressed)
			aForceAmplitude += 1.0;

		mySystem->Update(aForceAmplitude, Core::TimeModule::GetInstance()->GetDeltaTimeSec());
	}
	else if (aType == Core::Module::UpdateType::MainUpdate)
	{
		myGui->Update();
	}
}

void NeatDoublePoleBalancingModule::OnGuiUpdate()
{
	mySystem->Draw();
	ImGui::Text("Manual control");
}

int main()
{
	InitMemoryLeaksDetection();

	Core::Facade::Create(__argc, __argv);

	Render::RenderModule::Register();
	NeatDoublePoleBalancingModule::Register();

	Core::Facade::GetInstance()->Run(NeatDoublePoleBalancingModule::GetInstance()->GetWindow());

	NeatDoublePoleBalancingModule::Unregister();
	Render::RenderModule::Unregister();

	Core::Facade::Destroy();

	return EXIT_SUCCESS;
}
