#pragma once
class Efficiency
{
	double koeff;
public:
	Efficiency(){}
	Efficiency(double k): koeff(k){}
	// return value of efficiency function in a point
	double EfficiencyFunction(double x);
	// return value of efficiency by period
	double EfficiencyByPeriod(int busyCores, int tBegin, int tEnd);
	~Efficiency(void);
};

