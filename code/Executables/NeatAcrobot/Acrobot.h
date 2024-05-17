#pragma once

class Acrobot
{
public:
	Acrobot();
	void Reset();
	void Update(double aForceAmplitude, double aDeltaTime);
	void Draw();

	inline double GetPole1Angle() const { return std::atan2(std::sin(myState[0]), std::cos(myState[0])); }
	inline double GetPole2Angle() const { return std::atan2(std::sin(myState[1]), std::cos(myState[1])); }
	inline double GetPole1Velocity() const { return myState[2]; }
	inline double GetPole2Velocity() const { return myState[3]; }

	bool ArePolesUp() const;
	bool IsPole1Down() const;
	bool IsPole2Down() const;

private:
	void Step(double* dydt, double* y, double aForce);

	// Runge - Kutta 4th order integration method
	void RK4(double aForce, double aDeltaTime);

	double myPole1Length = 1.0;
	double myPole2Length = 1.0;
	double myPole1Mass = 1.0;
	double myPole2Mass = 1.0;
	double myGravity = 9.81;
	double myInputForce = 10.0;
	double myPoleFriction = 0.0;
	double myPoleFailureAngle = 0.2094384 * 5.0;

	// 0 : Pole1Angle
	// 1 : Pole2Angle
	// 2 : Pole1Velocity
	// 3 : Pole2Velosity
	double myState[4];
	double myInitState[4];
};

typedef std::vector<Acrobot> Acrobots;
typedef std::vector<Acrobots> AcrobotPool;
