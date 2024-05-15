#include "CartPole.h"

#include "imgui_helpers.h"

#define PI 3.14159265358979323846

CartPole::CartPole()
{
	Reset();
}

void CartPole::Reset()
{
	myState[0] = 0.0;
	myState[1] = 0.0;
	myState[2] = 0.0;
	myState[3] = 0.0;
	myState[4] = 0.0;
	myState[5] = 0.0;
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

void CartPole::RK4(double aForce, double aDeltaTime)
{
	double stateTmp[6], derivs1[6], derivs2[6], derivs3[6];

	Step(aForce, myState, derivs1);
	derivs1[0] = myState[1];
	derivs1[2] = myState[3];
	derivs1[4] = myState[5];

	for (uint i = 0; i < 6; ++i)
	{
		stateTmp[i] = myState[i] + (aDeltaTime / 2.0) * derivs1[i];
	}

	Step(aForce, stateTmp, derivs2);
	derivs2[0] = stateTmp[1];
	derivs2[2] = stateTmp[3];
	derivs2[4] = stateTmp[5];

	for (uint i = 0; i < 6; ++i)
	{
		stateTmp[i] = myState[i] + (aDeltaTime / 2.0) * derivs2[i];
	}

	Step(aForce, stateTmp, derivs3);
	derivs3[0] = stateTmp[1];
	derivs3[2] = stateTmp[3];
	derivs3[4] = stateTmp[5];

	for (uint i = 0; i < 6; ++i)
	{
		stateTmp[i] = myState[i] + aDeltaTime * derivs3[i];
		derivs3[i] += derivs2[i];
	}

	Step(aForce, stateTmp, derivs2);
	derivs2[0] = stateTmp[1];
	derivs2[2] = stateTmp[3];
	derivs2[4] = stateTmp[5];

	for (uint i = 0; i < 6; ++i)
	{
		myState[i] = myState[i] + (aDeltaTime / 6.0) * (derivs1[i] + derivs2[i] + 2.0 * derivs3[i]);
	}
}
