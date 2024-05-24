#include "Character.h"

#include "imgui_helpers.h"

#define TRANSLATION_FORCE_AMPLITUDE 1000.f
#define TRANSLATION_FRICTION_COEFF 3.f
#define TRANSLATION_STOP_THRESHOLD 1.f

#define ROTATION_FORCE_AMPLITUDE 10.f
#define ROTATION_FRICTION_COEFF 3.f
#define ROTATION_STOP_THRESHOLD 0.01f

#define NICE_CHARACTERS_DISTANCE 300.f

Character::Character(const glm::vec2& aPosition, float aDirection, float aRadius, const glm::vec4& aColor)
	: myPosition(aPosition)
	, myDirection(aDirection)
	, myRadius(aRadius)
	, myColor(aColor)
{
	Update(0.f, 0.f, 0.f, 0.f);
}

void Character::Draw()
{
	ImVec2 size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 bodyPos = pos + size / 2.f + ImVec2(myPosition.x, -myPosition.y);
	draw_list->AddCircleFilled(bodyPos, myRadius, ImGui::ColorConvertFloat4ToU32(ImVec4(myColor.r, myColor.g, myColor.b, myColor.a)));

	ImVec2 eye1Pos = bodyPos + ImVec2(myForwardVec.x, -myForwardVec.y) * (myRadius - 1.f) + ImVec2(myRightVec.x, -myRightVec.y) * myRadius / 3.f;
	ImVec2 eye1Pos2 = bodyPos + ImVec2(myForwardVec.x, -myForwardVec.y) * (myRadius + 1.8f) + ImVec2(myRightVec.x, -myRightVec.y) * myRadius / 3.f;
	ImVec2 eye2Pos = bodyPos + ImVec2(myForwardVec.x, -myForwardVec.y) * (myRadius - 1.f) - ImVec2(myRightVec.x, -myRightVec.y) * myRadius / 3.f;
	ImVec2 eye2Pos2 = bodyPos + ImVec2(myForwardVec.x, -myForwardVec.y) * (myRadius + 1.8f) - ImVec2(myRightVec.x, -myRightVec.y) * myRadius / 3.f;
	draw_list->AddCircleFilled(eye1Pos, 5.f, 0xFFFFFFFF);
	draw_list->AddCircleFilled(eye1Pos2, 3.f, 0xFF000000);
	draw_list->AddCircleFilled(eye2Pos, 5.f, 0xFFFFFFFF);
	draw_list->AddCircleFilled(eye2Pos2, 3.f, 0xFF000000);
}

void Character::Update(float aDeltaTime, float aForwardForce, float aRightForce, float aRotationForce)
{
	glm::vec2 acceleration = myForwardVec * aForwardForce * TRANSLATION_FORCE_AMPLITUDE + myRightVec * aRightForce * TRANSLATION_FORCE_AMPLITUDE - myVelocity * TRANSLATION_FRICTION_COEFF;
	aRotationForce = aRotationForce * ROTATION_FORCE_AMPLITUDE - myAngularVelocity * ROTATION_FRICTION_COEFF;

	myPosition += aDeltaTime * myVelocity;
	myVelocity += aDeltaTime * acceleration;
	if (glm::length(myVelocity) < TRANSLATION_STOP_THRESHOLD)
		myVelocity = glm::vec2(0.f, 0.f);

	myDirection += aDeltaTime * myAngularVelocity;
	myForwardVec.x = std::sin(myDirection);
	myForwardVec.y = std::cos(myDirection);
	myRightVec.x = std::cos(myDirection);
	myRightVec.y = -std::sin(myDirection);
	myDirection = std::atan2(myForwardVec.x, myForwardVec.y);
	myAngularVelocity += aDeltaTime * aRotationForce;
	if (std::abs(myAngularVelocity) < ROTATION_STOP_THRESHOLD)
		myAngularVelocity = 0.f;
}

void Character::Reset(const glm::vec2& aPosition, float aDirection)
{
	myPosition = aPosition;
	myVelocity = glm::vec2(0.f, 0.f);
	myDirection = aDirection;
	myAngularVelocity = 0.f;
	Update(0.f, 0.f, 0.f, 0.f);
}

void Character::GetBrainInputs(const Character& anOther, float& aDistanceInfo, float& anAlignementInfo, float& anAimInfo) const
{
	glm::vec2 charToCharVec = myPosition - anOther.myPosition;
	aDistanceInfo = std::clamp(glm::length(charToCharVec) / NICE_CHARACTERS_DISTANCE - 1.f, -1.f, 1.f);
	
	charToCharVec = glm::normalize(charToCharVec);
	anAlignementInfo = std::atan2(charToCharVec.x * anOther.myForwardVec.y - anOther.myForwardVec.x * charToCharVec.y, charToCharVec.x * anOther.myForwardVec.x + charToCharVec.y * anOther.myForwardVec.y) / (float)std::numbers::pi;
	anAimInfo = std::atan2(-charToCharVec.x * myForwardVec.y - myForwardVec.x * -charToCharVec.y, -charToCharVec.x * myForwardVec.x + -charToCharVec.y * myForwardVec.y) / (float)std::numbers::pi;
}

float Character::ComputePositionFitness(const Character& anOther) const
{
	float fitness = 1.f;

	float distanceInfo;
	float alignementInfo;
	float aimInfo;
	GetBrainInputs(anOther, distanceInfo, alignementInfo, aimInfo);

	fitness *= std::exp(-10.f * std::abs(distanceInfo)); // Try to be at a reasonable distance from the player 
	fitness *= std::exp(-3.f * std::abs(alignementInfo)) - std::exp(-50.f * std::abs(alignementInfo)); // Don't be exactly aligned with the player (to avoid being shot)
	fitness *= std::exp(-3.f * std::abs(aimInfo)); // Try to aim at the player

	return fitness;
}
