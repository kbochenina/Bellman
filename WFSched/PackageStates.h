#include "boost/tuple/tuple.hpp"
#include <vector>

using namespace boost;
using namespace std;

typedef tuples::tuple<int,int,double> PackageState; // (type, core, level)

#pragma once
class PackageStates
{
	// package number
	int pNum;
	vector <PackageState> fullStates;
public:
	PackageStates(int p, vector <PackageState> fS);
	int GetType(int stateNum) {return fullStates[stateNum].get<0>();}
	int GetCore(int stateNum) {return fullStates[stateNum].get<1>();}
	double GetLevel(int stateNum) {return fullStates[stateNum].get<2>();}
	int GetNextStateNum(int currentState, int controlType, int controlCore);
	~PackageStates(void);
};

