#include "CartPole.h"
#include "EvolutionParams.h"
#include <random>
#include "imgui_helpers.h"

#define PI 3.14159265358979323846

DoubleCartPole::DoubleCartPole()
{
	myInitState[0] = 0.0;
	myInitState[1] = PI;
	myInitState[2] = PI;
	myInitState[3] = 0.0;
	myInitState[4] = 0.0;
	myInitState[5] = 0.0;
	Reset();
}

void DoubleCartPole::Reset()
{
	myState[0] = myInitState[0];
	myState[1] = myInitState[1];
	myState[2] = myInitState[2];
	myState[3] = myInitState[3];
	myState[4] = myInitState[4];
	myState[5] = myInitState[5];
}

void DoubleCartPole::Update(double aForceAmplitude, double aDeltaTime)
{
	RK4(aForceAmplitude * myInputForce, aDeltaTime);
}

void DoubleCartPole::Draw()
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
	ImVec2 pole1EndPos = cartPos + ImVec2(pole1Size * static_cast<float>(std::sin(myState[1])), -pole1Size * static_cast<float>(std::cos(myState[1])));

	float pole2Size = size.x * static_cast<float>(2.0 * myPole2Length / windowWidthPhysical);
	ImVec2 pole2EndPos = cartPos + ImVec2(pole2Size * static_cast<float>(std::sin(myState[2])), -pole2Size * static_cast<float>(std::cos(myState[2])));

	draw_list->AddRectFilled(trackPos - trackSize / 2.f, trackPos + trackSize / 2.f, 0xFF00FF00);
	draw_list->AddRectFilled(cartPos - cartSize / 2.f, cartPos + cartSize / 2.f, 0xFFFFFFFF);
	draw_list->AddLine(cartPos, pole1EndPos, 0xFFFF0000, 5.f);
	draw_list->AddLine(cartPos, pole2EndPos, 0xFF0000FF, 5.f);
}

void DoubleCartPole::Step(double* dydt, double* y, double aForce)
{
	dydt[0] = y[3];
	dydt[1] = y[4];
	dydt[2] = y[5];

	double sin1 = std::sin(y[1]);
	double cos1 = std::cos(y[1]);
	double sin2 = std::sin(y[2]);
	double cos2 = std::cos(y[2]);

	static const double MUP = 0.000002;
	double tmp1 = MUP * y[4] / (myPole1Length * myPole1Mass);
	double tmp2 = MUP * y[5] / (myPole2Length * myPole2Mass);

	double f1 = (myPole1Length * myPole1Mass * y[4] * y[4] * sin1) +
		(0.75 * myPole1Mass * cos1 * (tmp1 + myGravity * sin1));
	double f2 = (myPole2Length * myPole2Mass * y[5] * y[5] * sin2) +
		(0.75 * myPole2Mass * cos2 * (tmp2 + myGravity * sin2));

	double m1 = myPole1Mass * (1.0 - (0.75 * cos1 * cos1));
	double m2 = myPole2Mass * (1.0 - (0.75 * cos2 * cos2));

	dydt[3] = (aForce + f1 + f2) / (m1 + m2 + myCartMass);
	dydt[4] = -0.75 * (dydt[3] * cos1 + myGravity * sin1 + tmp1) / myPole1Length;
	dydt[5] = -0.75 * (dydt[3] * cos2 + myGravity * sin2 + tmp2) / myPole2Length;
}

void DoubleCartPole::RK4(double aForce, double aDeltaTime)
{
	double k1[6], k2[6], k3[6], k4[6], tmp[6];

	Step(k1, myState, aForce);

	for (uint i = 0; i < 6; ++i)
		tmp[i] = myState[i] + (aDeltaTime / 2.0) * k1[i];
	Step(k2, tmp, aForce);

	for (uint i = 0; i < 6; ++i)
		tmp[i] = myState[i] + (aDeltaTime / 2.0) * k2[i];
	Step(k3, tmp, aForce);

	for (uint i = 0; i < 6; ++i)
		tmp[i] = myState[i] + aDeltaTime * k3[i];
	Step(k4, tmp, aForce);

	for (uint i = 0; i < 6; ++i)
		myState[i] = myState[i] + (aDeltaTime / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
}

DoubleCartPole2::DoubleCartPole2()
{
	myInitState[0] = 0.0;
	myInitState[1] = PI;
	myInitState[2] = PI;
	myInitState[3] = 0.0;
	myInitState[4] = 0.0;
	myInitState[5] = 0.0;
	Reset();
}

void DoubleCartPole2::Reset()
{
	myState[0] = myInitState[0];
	myState[1] = myInitState[1];
	myState[2] = myInitState[2];
	myState[3] = myInitState[3];
	myState[4] = myInitState[4];
	myState[5] = myInitState[5];
}

void DoubleCartPole2::Update(double aForceAmplitude, double aDeltaTime)
{
	RK4(aForceAmplitude * myInputForce, aDeltaTime);
}

void DoubleCartPole2::Draw()
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
	ImVec2 pole1EndPos = cartPos + ImVec2(pole1Size * static_cast<float>(std::sin(myState[1])), -pole1Size * static_cast<float>(std::cos(myState[1])));

	float pole2Size = size.x * static_cast<float>(2.0 * myPole2Length / windowWidthPhysical);
	ImVec2 pole2EndPos = pole1EndPos + ImVec2(pole2Size * static_cast<float>(std::sin(myState[2])), -pole2Size * static_cast<float>(std::cos(myState[2])));

	draw_list->AddRectFilled(trackPos - trackSize / 2.f, trackPos + trackSize / 2.f, 0xFF00FF00);
	draw_list->AddRectFilled(cartPos - cartSize / 2.f, cartPos + cartSize / 2.f, 0xFFFFFFFF);
	draw_list->AddLine(cartPos, pole1EndPos, 0xFFFF0000, 5.f);
	draw_list->AddLine(pole1EndPos, pole2EndPos, 0xFF0000FF, 5.f);
}

void DoubleCartPole2::Step(double* dydt, double* y, double aForce)
{
	dydt[0] = y[3];
	dydt[1] = y[4];
	dydt[2] = y[5];

	double z1 = myCartMass + myPole1Mass + myPole2Mass;
	double z2 = myPole1Mass * myPole1Length + myPole2Mass * 2.0 * myPole1Length;
	double z3 = myPole2Mass * myPole2Length;
	double z4 = ((myPole1Mass / 3.0) + myPole2Mass) * (2.0 * myPole1Length) * (2.0 * myPole1Length);
	double z5 = myPole2Mass * 2.0 * myPole1Length * myPole2Length;
	double z6 = myPole2Mass * (2.0 * myPole2Length) * (2.0 * myPole2Length) / 3.0;
	double f1 = z2 * myGravity;
	double f2 = z3 * myGravity;

	double sin1 = std::sin(y[1]);
	double cos1 = std::cos(y[1]);
	double sin2 = std::sin(y[2]);
	double cos2 = std::cos(y[2]);
	double sin1m2 = std::sin(y[1] - y[2]);
	double cos1m2 = std::cos(y[1] - y[2]);

	glm::dvec3 dy = { y[3], y[4], y[5] };

	glm::dmat3x3 M = {
		z1        , z2 * cos1   , z3 * cos2   ,
		z2 * cos1 , z4          , z5 * cos1m2 ,
		z3 * cos2 , z5 * cos1m2 , z6
	};
	glm::dmat3x3 Minv = glm::inverse(M);

	glm::dmat3x3 C = {
		0 , -z2 * sin1 * y[4]   , -z3 * sin2 * y[5]  ,
		0 , 0                   , z5 * sin1m2 * y[5] ,
		0 , -z5 * sin1m2 * y[4] , 0
	};

	glm::dvec3 G = { 0, -f1 * sin1, -f2 * sin2 };

	glm::dvec3 H = { aForce, 0, 0 };

	glm::dvec3 ddy = -Minv * C * dy - Minv * G + Minv * H;

	dydt[3] = ddy[0];
	dydt[4] = ddy[1] - myPoleFriction * dydt[1];
	dydt[5] = ddy[2] - myPoleFriction * dydt[2];
}

void DoubleCartPole2::RK4(double aForce, double aDeltaTime)
{
	double k1[6], k2[6], k3[6], k4[6], tmp[6];

	Step(k1, myState, aForce);

	for (uint i = 0; i < 6; ++i)
		tmp[i] = myState[i] + (aDeltaTime / 2.0) * k1[i];
	Step(k2, tmp, aForce);

	for (uint i = 0; i < 6; ++i)
		tmp[i] = myState[i] + (aDeltaTime / 2.0) * k2[i];
	Step(k3, tmp, aForce);

	for (uint i = 0; i < 6; ++i)
		tmp[i] = myState[i] + aDeltaTime * k3[i];
	Step(k4, tmp, aForce);

	for (uint i = 0; i < 6; ++i)
		myState[i] = myState[i] + (aDeltaTime / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
}
