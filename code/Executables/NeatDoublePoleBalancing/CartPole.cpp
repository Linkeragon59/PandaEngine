#include "CartPole.h"
#include "EvolutionParams.h"
#include <random>
#include "imgui_helpers.h"

#define PI 3.14159265358979323846

CartPole::CartPole(bool aRandomize)
{
	myInitState[0] = 0.0;
	myInitState[1] = 0.0;
	myInitState[2] = PI;
	myInitState[3] = 0.0;
	myInitState[4] = PI;
	myInitState[5] = 0.0;
	if (aRandomize)
	{
		std::uniform_real_distribution<> rand(-1.0, 1.0);
		myInitState[0] += rand(Neat::EvolutionParams::GetRandomGenerator()) * 0.001;
		//myInitState[1] += rand(Neat::EvolutionParams::GetRandomGenerator()) * 0.001;
		myInitState[2] += rand(Neat::EvolutionParams::GetRandomGenerator()) * PI / 100.f;
		//myInitState[3] += rand(Neat::EvolutionParams::GetRandomGenerator()) * 0.001;
		myInitState[4] += rand(Neat::EvolutionParams::GetRandomGenerator()) * PI / 100.f;
		//myInitState[5] += rand(Neat::EvolutionParams::GetRandomGenerator()) * 0.001;
	}
	Reset();
}

void CartPole::Reset()
{
	myState[0] = myInitState[0];
	myState[1] = myInitState[1];
	myState[2] = myInitState[2];
	myState[3] = myInitState[3];
	myState[4] = myInitState[4];
	myState[5] = myInitState[5];
}

void CartPole::Update(double aForceAmplitude, double aDeltaTime)
{
	double force = aForceAmplitude * myInputForce;
	for (uint i = 0; i < 2; ++i)
	{
		RK4(force, aDeltaTime);
	}
}

void CartPole::Draw()
{
	static const double windowWidthPhysical = 10.0;

	ImVec2 size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 trackSize = ImVec2(size.x * static_cast<float>(2.0 * myCartTrackSize / windowWidthPhysical), 20.f);
	ImVec2 trackPos = pos + ImVec2(size.x / 2.f, size.y / 2.f);

	ImVec2 cartSize = ImVec2(50.f, 30.f);
	ImVec2 cartPos = pos + ImVec2(size.x / 2.f + size.x * static_cast<float>(myState[0] / windowWidthPhysical), size.y / 2.f);

	float pole1Size = size.x * static_cast<float>(2.0 * myPole1Length / windowWidthPhysical);
	ImVec2 pole1EndPos = cartPos + ImVec2(pole1Size * static_cast<float>(std::sin(myState[2])), -pole1Size * static_cast<float>(std::cos(myState[2])));

	float pole2Size = size.x * static_cast<float>(2.0 * myPole2Length / windowWidthPhysical);
	ImVec2 pole2EndPos = cartPos + ImVec2(pole2Size * static_cast<float>(std::sin(myState[4])), -pole2Size * static_cast<float>(std::cos(myState[4])));

	draw_list->AddRectFilled(trackPos - trackSize / 2.f, trackPos + trackSize / 2.f, 0xFF00FF00);
	draw_list->AddRectFilled(cartPos - cartSize / 2.f, cartPos + cartSize / 2.f, 0xFFFFFFFF);
	draw_list->AddLine(cartPos, pole1EndPos, 0xFFFF0000, 5.f);
	draw_list->AddLine(cartPos, pole2EndPos, 0xFF0000FF, 5.f);
}

bool CartPole::ArePolesUp() const
{
	if (std::abs(GetPole1Angle()) > myPoleFailureAngle)
		return false;
	if (std::abs(GetPole2Angle()) > myPoleFailureAngle)
		return false;
	return true;
}

bool CartPole::IsSlowAndCentered() const
{
	if (std::abs(myState[0]) > myCartTrackSize / 10.0)
		return false;
	if (std::abs(myState[1]) > 1.0)
		return false;
	if (std::abs(myState[3]) > 1.0)
		return false;
	if (std::abs(myState[5]) > 1.0)
		return false;
	return true;
}

void CartPole::Step(double aForce, double* aState, double* aDerivs)
{
	aDerivs[0] = aState[1];
	aDerivs[2] = aState[3];
	aDerivs[4] = aState[5];

	double sinAngle1 = std::sin(aState[2]);
	double cosAngle1 = std::cos(aState[2]);
	double sinAngle2 = std::sin(aState[4]);
	double cosAngle2 = std::cos(aState[4]);

	static const double MUP = 0.000002;
	double tmp1 = MUP * aState[3] / (myPole1Length * myPole1Mass);
	double tmp2 = MUP * aState[5] / (myPole2Length * myPole2Mass);

	double f1 = (myPole1Length * myPole1Mass * aState[3] * aState[3] * sinAngle1) +
		(0.75 * myPole1Mass * cosAngle1 * (tmp1 + myGravity * sinAngle1));
	double f2 = (myPole2Length * myPole2Mass * aState[5] * aState[5] * sinAngle2) +
		(0.75 * myPole2Mass * cosAngle2 * (tmp2 + myGravity * sinAngle2));

	double m1 = myPole1Mass * (1.0 - (0.75 * cosAngle1 * cosAngle1));
	double m2 = myPole2Mass * (1.0 - (0.75 * cosAngle2 * cosAngle2));

	aDerivs[1] = (aForce + f1 + f2) / (m1 + m2 + myCartMass);
	aDerivs[3] = -0.75 * (aDerivs[1] * cosAngle1 + myGravity * sinAngle1 + tmp1) / myPole1Length;
	aDerivs[5] = -0.75 * (aDerivs[1] * cosAngle2 + myGravity * sinAngle2 + tmp2) / myPole2Length;
}

void CartPole::Step2(double aForce, double* aState, double* aDerivs)
{
	aDerivs[0] = aState[1];
	aDerivs[2] = aState[3];
	aDerivs[4] = aState[5];

	aDerivs[1] = (4.0 * aForce * std::cos(2.0 * aState[4])
		- 6.0 * aForce
		+ 4.0 * aDerivs[2] * aDerivs[2] * std::cos(aState[2])
		+ aDerivs[2] * aDerivs[2] * std::cos(aState[2] - aState[4])
		- aDerivs[2] * aDerivs[2] * std::cos(aState[2] + 2.0 * aState[4])
		+ 2.0 * aDerivs[2] * aDerivs[4] * std::cos(aState[2] - aState[4])
		+ aDerivs[4] * aDerivs[4] * std::cos(aState[2] - aState[4])
		- 29.43 * std::sin(2.0 * aState[2])
		+ 9.81 * std::sin(2.0 * aState[2] + 2.0 * aState[4]))
		/ (3.0 * std::cos(2.0 * aState[2])
			- 22.0 * std::cos(2.0 * aState[4])
			- std::cos(2.0 * aState[2] + 2.0 * aState[4])
			+ 34.0);
	aDerivs[3] = (-8.0 * aForce * std::sin(aState[2])
		+ 4.0 * aForce * std::sin(aState[2] + 2.0 * aState[4])
		+ 3.0 * aDerivs[2] * aDerivs[2] * std::sin(2.0 * aState[2])
		+ 23.0 * aDerivs[2] * aDerivs[2] * std::sin(aState[4])
		+ 22.0 * aDerivs[2] * aDerivs[2] * std::sin(2.0 * aState[4])
		+ aDerivs[2] * aDerivs[2] * std::sin(2.0 * aState[2] + aState[4])
		+ 46.0 * aDerivs[2] * aDerivs[4] * std::sin(aState[4])
		+ 2.0 * aDerivs[2] * aDerivs[4] * std::sin(2.0 * aState[2] + aState[4])
		+ 23.0 * aDerivs[4] * aDerivs[4] * std::sin(aState[4])
		+ aDerivs[4] * aDerivs[4] * std::sin(2.0 * aState[2] + aState[4])
		- 490.5 * std::cos(aState[2])
		+ 215.82 * std::cos(aState[2] + 2.0 * aState[4]))
		/ (3.0 * std::cos(2.0 * aState[2])
			- 22.0 * std::cos(2.0 * aState[4])
			- std::cos(2.0 * aState[2] + 2.0 * aState[4])
			+ 34.0);
	aDerivs[5] = -((100.0 * aDerivs[2] * aDerivs[2] * std::sin(aState[4]) + 981.0 * std::cos(aState[2] + aState[4]))
		* (-std::pow(3.0 * std::sin(aState[2]) + std::sin(aState[2] + aState[4]), 2.0) + 28.0 * std::cos(aState[4]) + 42.0)
		+ 0.5 * (200.0 * aDerivs[2] * aDerivs[4] * std::sin(aState[4])
			+ 100.0 * aDerivs[4] * aDerivs[4] * std::sin(aState[4])
			- 2943.0 * std::cos(aState[2])
			- 981.0 * std::cos(aState[2] + aState[4]))
		* (25.0 * std::cos(aState[4]) + 3.0 * std::cos(2.0 * aState[2] + aState[4]) + std::cos(2.0 * aState[2] + 2.0 * aState[4]) + 13.0)
		+ 50.0 * (2.0 * std::sin(aState[2]) + 3.0 * std::sin(aState[2] - aState[4])
			- 2.0 * std::sin(aState[2] + aState[4]) - std::sin(aState[2] + 2.0 * aState[4]))
		* (-2.0 * aForce + 3.0 * aDerivs[2] * aDerivs[2] * std::cos(aState[2])
			+ aDerivs[2] * aDerivs[2] * std::cos(aState[2] + aState[4])
			+ 2.0 * aDerivs[2] * aDerivs[4] * std::cos(aState[2] + aState[4])
			+ aDerivs[4] * aDerivs[4] * std::cos(aState[2] + aState[4])))
		/ (75.0 * std::cos(2.0 * aState[2])
			- 550.0 * std::cos(2.0 * aState[4])
			- 25.0 * std::cos(2.0 * aState[2] + 2.0 * aState[4])
			+ 850.0);
}

void CartPole::RK4(double aForce, double aDeltaTime)
{
	double stateTmp[6], derivs1[6], derivs2[6], derivs3[6];

	Step(aForce, myState, derivs1);

	for (uint i = 0; i < 6; ++i)
	{
		stateTmp[i] = myState[i] + (aDeltaTime / 2.0) * derivs1[i];
	}

	Step(aForce, stateTmp, derivs2);

	for (uint i = 0; i < 6; ++i)
	{
		stateTmp[i] = myState[i] + (aDeltaTime / 2.0) * derivs2[i];
	}

	Step(aForce, stateTmp, derivs3);

	for (uint i = 0; i < 6; ++i)
	{
		stateTmp[i] = myState[i] + aDeltaTime * derivs3[i];
		derivs3[i] += derivs2[i];
	}

	Step(aForce, stateTmp, derivs2);

	for (uint i = 0; i < 6; ++i)
	{
		myState[i] = myState[i] + (aDeltaTime / 6.0) * (derivs1[i] + derivs2[i] + 2.0 * derivs3[i]);
	}
}
