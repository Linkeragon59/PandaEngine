#pragma once

#define PI 3.14159265358979323846

class Acrobot
{
public:
	Acrobot(bool aStartUp, double aVariance);
	void Reset();
	void Update(double aForceAmplitude, double aDeltaTime);
	void Draw();

	inline double GetPole1Angle() const { return std::atan2(std::sin(PI - myState[0]), std::cos(PI - myState[0])); }
	inline double GetPole2Angle() const { return std::atan2(std::sin(PI - myState[0] + myState[1]), std::cos(PI - myState[0] + myState[1])); }
	inline double GetPole1Velocity() const { return myState[2]; }
	inline double GetPole2Velocity() const { return myState[3]; }

	bool ArePolesUp() const;
	bool IsPole1Up() const;
	bool IsPole2Up() const;
	bool ArePolesSlow() const;

private:
	void Step(double* dydt, double* y, double aForce);

	// Runge - Kutta 4th order integration method
	void RK4(double aForce, double aDeltaTime);

	double myPole1Length = 1.0;
	double myPole2Length = 1.0;
	double myPole1Mass = 1.0;
	double myPole2Mass = 1.0;
	double myGravity = 9.81;
	double myInputForce = 15.0;
	double myPoleFriction = 0.0;
	double myPoleFailureAngle = 0.2094384 * 5.0;

	// 0 : Pole1Angle - Angle to down
	// 1 : Pole2Angle - Angle to pole 1
	// 2 : Pole1Velocity
	// 3 : Pole2Velosity
	double myState[4];
	double myInitState[4];
};

typedef std::vector<Acrobot> Acrobots;
typedef std::vector<Acrobots> AcrobotPool;
