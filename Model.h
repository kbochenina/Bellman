#pragma once


#include "Workflow.h"
#include "ResourceType.h"



typedef pair <int, double> StateInformation; // (number of uopt, max eff on this state)
typedef vector <StateInformation> StageInformation;
typedef vector <StageInformation> BackBellmanInfo;
typedef vector<vector<pair<double, unsigned int>>> timeCore;

extern int T, delta, stages;
extern vector<int> stageBorders;

class Model
{
	
	// fields
	vector <Workflow*> Workflows;
	std::vector <ResourceType*> Resources;
	vector <tuple<int,int,vector<int>>> stagesCores; // (package,stage, core1, core2,...) for package1, (stage, core1, core2, ...) for package 2...
	std::vector <int> forcedBricks;
	bool canExecuteOnDiffResources; // if true one package can execute on some cores of different resources of one type
	double koeff;
	vector <pair<unsigned short, unsigned short>> typesCores;
	// for each state: vector of controls. Each package control can be -1 (if no resources used)
	// or index in typesCores
	vector <vector<vector<int>>> controls; 
	vector <vector<int>> nextStateNumbers;
	vector <vector<int>> states; // (stateNum for P1, stateNum for P2, ...)
	BackBellmanInfo FullInfo;
	StageInformation StageTable; //?
	double maxEfficiency; //?
	// methods
	void InitResources(string);
	void InitWorkflows(string);
	void InitSettings(string);
	bool CheckState (const unsigned int state, const unsigned int stage,timeCore&);
	bool CheckControl(const unsigned int &state, const unsigned int &control, const unsigned int &stage, timeCore&, vector <vector<int>>&fullUsedNums, bool isUsedNumsNeeded);
	void SetForcedBricks();
	double efficiencyFunction(double x) { return (koeff*(1-x)); }
	double EfficiencyByPeriod(int busyCores, int t1, int t2){
		return ( busyCores * (double)(t2-t1)/(double)T * (efficiencyFunction((double)t1/(double)T) + efficiencyFunction((double)t2/(double)T)) / 2  );
	}
	int GetStatesCount() {return states.size();}
	double GetEfficiency(const int & stage, const timeCore& currentTC);
public:
	Model(){}
	void Init (string resFile, string wfFile, string settingsFile);

	// debug and time measurement functions
	double Greedy(int uopt, double currentEff);
	void GetStageInformation(int stage);
	//void CheckForIllegalCoreNumber(vector<vector <OnePackageControl>> &fullPossibleControls);
	//PossibleControls GetStateControls(int stateNumber, ofstream& file);
	void GetOneStageControls(int tbegin);
	void OneStep(int periodNumber);
	
	//int GetBusyCores(OneControl& control);
	//int GetStateNumber(vector <PackageState> &resultState, int stateNum);
	
	void DirectBellman();
	
	//PossibleControls CheckControls(int i, int);
	int GetTRealBegin(int l, double level, int periodBegin);
	/*double GetMaxEfficiency(){
		double eff = 0.0;
		for (int i = 0; i < Nodes.size(); i++){
			multimap <int,pair<int,int>> bi = Nodes[i]->GetBusyIntervals();
			multimap <int,pair<int,int>>::iterator bii = bi.begin();
			for (; bii!=bi.end(); bii++)
			{
				int numCores = (*bii).first;
				int tb = (*bii).second.first;
				int te = (*bii).second.second;
				eff += EfficiencyByPeriod(numCores,tb,te);
			}
		}
		maxEfficiency = 1-eff;
		return maxEfficiency;
	}*/
	~Model(void);
};

