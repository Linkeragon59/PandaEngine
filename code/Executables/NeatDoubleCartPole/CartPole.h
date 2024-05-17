#pragma once

class DoubleCartPoleBase
{
public:
	DoubleCartPoleBase();
	virtual ~DoubleCartPoleBase() {}
	void Reset();
	void Update(double aForceAmplitude, double aDeltaTime);
	virtual void Draw() = 0;

	inline double GetCartPosition() const { return myState[0]; }
	inline double GetPole1Angle() const { return std::atan2(std::sin(myState[1]), std::cos(myState[1])); }
	inline double GetPole2Angle() const { return std::atan2(std::sin(myState[2]), std::cos(myState[2])); }
	inline double GetCartVelocity() const { return myState[3]; }
	inline double GetPole1Velocity() const { return myState[4]; }
	inline double GetPole2Velocity() const { return myState[5]; }

protected:
	virtual void Step(double* dydt, double* y, double aForce) = 0;

	// Runge - Kutta 4th order integration method
	void RK4(double aForce, double aDeltaTime);

	double myCartTrackSize = 2.5;
	double myPole1Length = 0.5; // Half
	double myPole2Length = 0.25; // Half
	double myCartMass = 1.0;
	double myPole1Mass = 0.1;
	double myPole2Mass = 0.05;
	double myGravity = 9.81;
	double myInputForce = 10.0;
	double myPoleFailureAngle = 0.628329;
	double myPoleFriction = 0.05;

	// 0 : CartPosistion
	// 1 : Pole1Angle
	// 2 : Pole2Angle
	// 3 : CartVelocity
	// 4 : Pole1Velocity
	// 5 : Pole2Velosity
	double myState[6];
	double myInitState[6];

};

class DoubleCartPole : public DoubleCartPoleBase
{
public:
	void Draw() override;

private:
	void Step(double* dydt, double* y, double aForce) override;
};

class DoubleCartPole2 : public DoubleCartPoleBase
{
public:
	void Draw() override;

private:
	void Step(double* dydt, double* y, double aForce) override;
};
