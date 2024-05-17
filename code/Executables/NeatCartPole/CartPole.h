#pragma once

#define PI 3.14159265358979323846

struct CartPole
{
	CartPole(double aStartPoleAngle, double aVariance, bool aHighStartVelocities);
	void Reset();

	void Update(double aForce, double aDeltaTime);
	double GetPoleAngle() const { return std::atan2(std::sin(myPoleAngle), std::cos(myPoleAngle)); }
	bool IsPoleUp() const { return std::abs(GetPoleAngle()) <= myPoleFailureAngle; }
	bool IsSlowAndCentered() const;
	void Draw(const glm::vec2& aMousePos);

	double myInitPoleAngle = 0.0;
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

typedef std::vector<CartPole> CartPoles;
typedef std::vector<CartPoles> CartPolePool;
