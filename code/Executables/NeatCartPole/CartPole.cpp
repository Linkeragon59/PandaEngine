#include "CartPole.h"
#include "EvolutionParams.h"
#include <random>
#include "imgui_helpers.h"
#include <format>

CartPole::CartPole(double aStartPoleAngle, double aVariance, bool aHighStartVelocities)
{
	myInitPoleAngle = aStartPoleAngle;
	if (aVariance > DBL_EPSILON)
	{
		std::uniform_real_distribution<> rand(-aVariance, aVariance);
		myInitPoleAngle += rand(Neat::EvolutionParams::GetRandomGenerator()) * PI;
		myInitCartPosition = rand(Neat::EvolutionParams::GetRandomGenerator()) * myCartTrackSize;
		myInitPoleVelocity = rand(Neat::EvolutionParams::GetRandomGenerator()) * (aHighStartVelocities ? 10.0 : 3.0);
		myInitCartVelocity = rand(Neat::EvolutionParams::GetRandomGenerator()) * (aHighStartVelocities ? 10.0 : 3.0);
	}
	Reset();
}

void CartPole::Reset()
{
	myPoleAngle = myInitPoleAngle;
	myPoleVelocity = myInitPoleVelocity;
	myCartPosition = myInitCartPosition;
	myCartVelocity = myInitCartVelocity;
}

void CartPole::Update(double aForceAmplitude, double aDeltaTime)
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

bool CartPole::IsSlowAndCentered() const
{
	if (std::abs(myCartPosition) > myCartTrackSize / 10.0)
		return false;

	if (std::abs(myCartVelocity) > 1.0)
		return false;

	if (std::abs(myPoleVelocity) > 1.0)
		return false;

	return true;
}

void CartPole::Draw(const glm::vec2& aMousePos)
{
	static const double windowWidthPhysical = 10.0;

	ImVec2 size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 trackSize = ImVec2(size.x * static_cast<float>(2.0 * myCartTrackSize / windowWidthPhysical), 20.f);
	ImVec2 trackPos = pos + ImVec2(size.x / 2.f, size.y / 2.f);

	ImVec2 cartSize = ImVec2(50.f, 30.f);
	ImVec2 cartPos = pos + ImVec2(size.x / 2.f + size.x * static_cast<float>(myCartPosition / windowWidthPhysical), size.y / 2.f);

	float poleSize = size.x * static_cast<float>(2.0 * myPoleLength / windowWidthPhysical);
	ImVec2 poleEndPos = cartPos + ImVec2(poleSize * static_cast<float>(std::sin(myPoleAngle)), -poleSize * static_cast<float>(std::cos(myPoleAngle)));
	glm::vec2 poleExtremityPos = glm::vec2(poleEndPos.x, poleEndPos.y);
	while (glm::length(aMousePos - poleExtremityPos) < 50.f)
	{
		glm::vec2 poleVec = poleExtremityPos - glm::vec2(cartPos.x, cartPos.y);
		glm::vec2 mouseVec = aMousePos - glm::vec2(cartPos.x, cartPos.y);
		float mousePoleAngle = std::atan2(mouseVec.x * poleVec.y - poleVec.x * mouseVec.y, mouseVec.x * poleVec.x + mouseVec.y * poleVec.y);
		myPoleAngle += mousePoleAngle > 0.f ? 0.01 : -0.01;
		myPoleVelocity = 0.0;
		poleEndPos = cartPos + ImVec2(poleSize * static_cast<float>(std::sin(myPoleAngle)), -poleSize * static_cast<float>(std::cos(myPoleAngle)));
		poleExtremityPos = glm::vec2(poleEndPos.x, poleEndPos.y);
	}

	ImU32 trackColor = IsPoleUp() ? 0xFF00FF00 : 0xFF0000FF;

	draw_list->AddRectFilled(trackPos - trackSize / 2.f, trackPos + trackSize / 2.f, trackColor);
	draw_list->AddRectFilled(cartPos - cartSize / 2.f, cartPos + cartSize / 2.f, 0xFFFFFFFF);
	draw_list->AddLine(cartPos, poleEndPos, 0xFFFF0000, 5.f);

	double angleDegrees = GetPoleAngle() * 180.0 / PI;
	draw_list->AddText(trackPos + ImVec2(0.f, 50.f), 0xFFFFFFFF, std::format("Angle : {}", angleDegrees).c_str());
	draw_list->AddText(trackPos + ImVec2(0.f, 75.f), 0xFFFFFFFF, std::format("Position : {}", myCartPosition).c_str());
	draw_list->AddText(trackPos + ImVec2(0.f, 100.f), 0xFFFFFFFF, std::format("Pole Velocity : {}", myPoleVelocity).c_str());
	draw_list->AddText(trackPos + ImVec2(0.f, 125.f), 0xFFFFFFFF, std::format("Cart Velocity : {}", myCartVelocity).c_str());

	draw_list->AddCircleFilled(poleEndPos, 5.f, 0xFFFF0000);
	draw_list->AddCircleFilled(ImVec2(aMousePos.x, aMousePos.y), 5.f, 0xFFFF0000);
	draw_list->AddCircle(ImVec2(aMousePos.x, aMousePos.y), 50.f, 0xFFFF0000);
}
