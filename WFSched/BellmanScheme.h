#include "SchedulingMethod.h"
#include "PackageStates.h"

typedef pair <int, double> StateInformation; // (number of optimal control, max eff on this state)
typedef vector <StateInformation> StageInformation;
typedef vector <StageInformation> BackBellmanInfo;
typedef vector <TimeCore> AllTimeCore;

#pragma once
class BellmanScheme : public SchedulingMethod
{
	// for each state: vector of package state numbers
	vector<vector<int>> states;
	// for each state: vector of package next state numbers?
	vector<vector<int>> nextStates;
	// vector of controls?
	vector<vector<vector<int>>> controls;
	// full info about back procedure results
	BackBellmanInfo fullInfo;
	// when WF produced in Bellman scheme for the first time,
	// we need to call SetData() function
	// otherwise call ReadData() function
	static vector <int> settedWFs;
	// vector of packages states
	vector <PackageStates> pStates;
// !!!??? see which methods are ACTUALLY public
public:
	BellmanScheme(DataInfo &d,int u, int w) : SchedulingMethod(d,u,w){};
	// common function for setting data
	void SetData();
	// setting packages states
	void SetPackagesStates();
	// recursive function for initializing states, nextStates and controls
	void SetFullData (int pNum);
	// recursive procedure for getting controls
	void GetControls();
	// reading full data from file
	void ReadData();
	void BackBellmanProcedure();
	void GetStageInformation(int stage);
	// awful function concretizing states or controls
	bool FindConcretization(int state, int control, int stage, 
		AllTimeCore timeCores, vector <vector<int>> packagesCoresNums);
	// additional eff function for using in GetStageInformation() function
	double GetEfficiency(int state, int control, int stage);
	// don't know about this?
	bool Check(int state, vector <int> otherStates, int pNum);
	void DirectBellman();
	double GetWFSchedule(Schedule &out);
	void printInfo();
	~BellmanScheme(void);
};

