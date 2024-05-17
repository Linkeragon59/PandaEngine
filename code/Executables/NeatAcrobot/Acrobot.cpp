#include "Acrobot.h"
#include "EvolutionParams.h"
#include <random>
#include "imgui_helpers.h"

#define PI 3.14159265358979323846

Acrobot::Acrobot()
{
	myInitState[0] = PI;
	myInitState[1] = 0.0;
	myInitState[2] = 0.0;
	myInitState[3] = 0.0;
	Reset();
}

void Acrobot::Reset()
{
	myState[0] = myInitState[0];
	myState[1] = myInitState[1];
	myState[2] = myInitState[2];
	myState[3] = myInitState[3];
}

void Acrobot::Update(double aForceAmplitude, double aDeltaTime)
{
	RK4(aForceAmplitude * myInputForce, aDeltaTime);
}

void Acrobot::Draw()
{
	static const double windowWidthPhysical = 10.0;

	ImVec2 size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 startPos = pos + ImVec2(size.x / 2.f, size.y / 2.f);

	float pole1Size = size.x * static_cast<float>(myPole1Length / windowWidthPhysical);
	ImVec2 pole1EndPos = startPos + ImVec2(pole1Size * static_cast<float>(std::sin(myState[0])), pole1Size * static_cast<float>(std::cos(myState[0])));

	float pole2Size = size.x * static_cast<float>(myPole2Length / windowWidthPhysical);
	ImVec2 pole2EndPos = pole1EndPos + ImVec2(pole2Size * static_cast<float>(std::sin(myState[0] + myState[1])), pole2Size * static_cast<float>(std::cos(myState[0] + myState[1])));

	ImColor color = 0xFF00FF00;
	if (!ArePolesUp())
		color = 0xFF0000FF;
	draw_list->AddCircleFilled(startPos, 5.f, color);
	draw_list->AddLine(startPos, pole1EndPos, 0xFFFF0000, 5.f);
	draw_list->AddLine(pole1EndPos, pole2EndPos, 0xFF0000FF, 5.f);
}

bool Acrobot::ArePolesUp() const
{
	if (std::abs(GetPole1Angle()) < PI - myPoleFailureAngle)
		return false;
	if (std::abs(GetPole2Angle()) > myPoleFailureAngle)
		return false;
	return true;
}

bool Acrobot::IsPole1Down() const
{
	if (std::abs(GetPole1Angle()) > myPoleFailureAngle)
		return false;
	return true;
}

bool Acrobot::IsPole2Down() const
{
	if (PI - std::abs(GetPole2Angle()) > myPoleFailureAngle)
		return false;
	return true;
}

void Acrobot::Step(double* dydt, double* y, double aForce)
{
	dydt[0] = y[2];
	dydt[1] = y[3];

	double sin1 = std::sin(y[0]);
	double sin2 = std::sin(y[1]);
	double cos2 = std::cos(y[1]);
	double sin1p2 = std::sin(y[0] + y[1]);

	double I1 = myPole1Mass * myPole1Length * myPole1Length;
	double I2 = myPole2Mass * myPole2Length * myPole2Length;

	double m1 = I1 + I2 + myPole2Mass * myPole1Length * myPole1Length + myPole2Mass * myPole1Length * myPole2Length * cos2;
	double m2 = I2 + myPole2Mass * myPole1Length * 0.5 * myPole2Length * cos2;
	double m3 = I2;
	double c1 = myPole2Mass * myPole1Length * 0.5 * myPole2Length * sin2;
	double f2 = myPole2Mass * myGravity * myPole2Length * sin1p2;
	double f1 = (myPole1Mass * 0.5 * myPole1Length + myPole2Mass * myPole1Length) * myGravity * sin1 + f2;

	glm::dvec2 dy = { y[2], y[3] };

	glm::dmat2x2 M = {
		m1 , m2 ,
		m2 , m3
	};
	glm::dmat2x2 Minv = glm::inverse(M);

	glm::dmat2x2 C = {
		-2.0 * c1 * y[3] , -c1 * y[3] ,
		c1 * y[2] , 0
	};

	glm::dvec2 G = { f1, f2 };

	glm::dvec2 H = { 0, aForce };

	glm::dvec2 ddy = -Minv * C * dy - Minv * G + Minv * H;

	dydt[2] = ddy[0] - myPoleFriction * dydt[0];
	dydt[3] = ddy[1] - myPoleFriction * dydt[1];
}

void Acrobot::RK4(double aForce, double aDeltaTime)
{
	double k1[4], k2[4], k3[4], k4[4], tmp[4];

	Step(k1, myState, aForce);

	for (uint i = 0; i < 4; ++i)
		tmp[i] = myState[i] + (aDeltaTime / 2.0) * k1[i];
	Step(k2, tmp, aForce);

	for (uint i = 0; i < 4; ++i)
		tmp[i] = myState[i] + (aDeltaTime / 2.0) * k2[i];
	Step(k3, tmp, aForce);

	for (uint i = 0; i < 4; ++i)
		tmp[i] = myState[i] + aDeltaTime * k3[i];
	Step(k4, tmp, aForce);

	for (uint i = 0; i < 4; ++i)
		myState[i] = myState[i] + (aDeltaTime / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
}
