#pragma once

class Character
{
public:
	Character(const glm::vec2& aPosition, float aDirection, float aRadius, const glm::vec4& aColor);

	void Draw();
	void Update(float aDeltaTime, float aForwardForce, float aRightForce, float aRotationForce);
	void Reset(const glm::vec2& aPosition, float aDirection);

	void GetBrainInputs(const Character& anOther, float& aDistanceInfo, float& anAlignementInfo, float& anAimInfo) const;

	float ComputePositionFitness(const Character& anOther) const;

private:
	glm::vec2 myPosition = { 0.f, 0.f };
	glm::vec2 myVelocity = { 0.f, 0.f };
	float myDirection = 0.f;
	float myAngularVelocity = 0.f;

	glm::vec2 myForwardVec = { 0.f, 1.f };
	glm::vec2 myRightVec = { 1.f, 0.f };

	float myRadius = 10.f;
	glm::vec4 myColor = { 1.f, 1.f, 1.f, 1.f };
};
