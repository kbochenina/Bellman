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
	// private methods
	// common function for setting data
	void SetData();
	// setting packages states
	void SetPackagesStates();
	// recursive function for initializing states
	void SetStates (int pNum);
	// procedure for setting controls
	void SetControls();
	// recursive function for getting controls for one state
	void GetControls(const vector<int>& state, int pNum);
	// check the state for consistency
	// (otherStates is already checked)
	bool CheckState(const int& state, const vector <int>& otherStates, int pNum);
	void CheckForReadiness(vector<int>& nextStates);
	// reading full data from file
	void ReadData();
	void BackBellmanProcedure();
	void GetStageInformation(int stage);
	// awful function concretizing states or controls
	bool FindConcretization(int state, int control, int stage, 
		AllTimeCore timeCores, vector <vector<int>> packagesCoresNums);
	// additional eff function for using in GetStageInformation() function
	double GetEfficiency(int state, int control, int stage);
	void DirectBellman();
	void printInfo();
public:
	BellmanScheme(DataInfo &d,int u, int w);
	double GetWFSchedule(Schedule &out);
	~BellmanScheme(void);
};

