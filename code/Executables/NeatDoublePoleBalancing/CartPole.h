#pragma once

class CartPole
{
public:
	CartPole(bool aRandomize);
	void Reset();
	void Update(double aForceAmplitude, double aDeltaTime);
	void Draw();

	double GetCartPosition() const { return myState[0]; }
	double GetCartVelocity() const { return myState[1]; }
	double GetPole1Angle() const { return std::atan2(std::sin(myState[2]), std::cos(myState[2])); }
	double GetPole1Velocity() const { return myState[3]; }
	double GetPole2Angle() const { return std::atan2(std::sin(myState[4]), std::cos(myState[4])); }
	double GetPole2Velocity() const { return myState[5]; }

	bool ArePolesUp() const;
	bool IsSlowAndCentered() const;

private:
	void Step(double aForce, double* aState, double* aDerivs);
	void Step2(double aForce, double* aState, double* aDerivs);

	// Runge - Kutta 4th order integration method
	void RK4(double aForce, double aDeltaTime);

	double myCartTrackSize = 2.5;
	double myPole1Length = 0.5;
	double myPole2Length = 0.25;
	double myCartMass = 1.0;
	double myPole1Mass = 0.1;
	double myPole2Mass = 0.05;
	double myGravity = -9.81;
	double myInputForce = 10.0;
	double myPoleFailureAngle = 0.628329;

	// 0 : CartPosistion
	// 1 : CartVelocity
	// 2 : Pole1Angle
	// 3 : Pole1Velocity
	// 4 : Pole2Angle
	// 5 : Pole2Velosity
	double myState[6];
	double myInitState[6];
};

typedef std::vector<CartPole> CartPoles;
typedef std::vector<CartPoles> CartPolePool;
