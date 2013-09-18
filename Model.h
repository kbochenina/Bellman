#pragma once
#include "Tree.h"
#include "Node.h"
#include "Workflow.h"
#include "Package.h"

typedef tuple <int,int,int> OnePackageControl; // (number of package, number of node, core count)
typedef vector <OnePackageControl> OneControl; // (number of package1, number of node, core count), ..., (number of packageN, number of node, core count)
typedef vector <OneControl> PossibleControls;
typedef vector <PossibleControls> Control;
typedef tuple <int,int,int,float> PackageState;
typedef vector <vector <PackageState>> State;
typedef map <int, vector <int>> NextStates;
typedef tuple <int, vector <int>, float> StateInformation; // (number of state, vector of uopts, max efficiency)
typedef vector <StateInformation> StageInformation;
typedef vector <StageInformation> BackBellmanInfo;
typedef multimap<int, pair<int,int> > WorkflowRepeatChanges; // (number of fullstate,number of state, repeatCountDecrement)


class Model
{
	vector <pair<int,int>> diff;
	float maxEfficiency ;
	int T; // number of seconds
	int delta; // length of time intervals
	vector <int> leftBorders;
	vector <int> freeCores;
	vector <Node*> Nodes;
	vector <Workflow*> Workflows;
	vector <Package*> Packages;
	vector <State> States;
	vector <int> repeatCounts;
	State FullState;
	Control FullControl;
	BackBellmanInfo FullInfo;
	StageInformation StageTable;
	vector <int> pControlNumbers;
	NextStates FullNextState;
	
public:
	int allCoresCount;
	float koeff;
	float Greedy(int uopt, float currentEff);
	void GetStageInformation(int stage);
	float efficiencyFunction(float x) { return (koeff*(1-x)); }
	float EfficiencyByPeriod(int busyCores, int t1, int t2){
		return ( busyCores * (float)(t2-t1)/(float)T * (efficiencyFunction((float)t1/(float)T) + efficiencyFunction((float)t2/(float)T)) / 2  );
	}
	void CheckForIllegalCoreNumber(vector<vector <OnePackageControl>> &fullPossibleControls);
	void SetFreeCores(int tbegin);
	Model(int t, int d){T = t; delta = d; for (int i = 0; i < T; i+= delta) leftBorders.push_back(i);}
	PossibleControls GetStateControls(int stateNumber, ofstream& file);
	void GetOneStageControls(int tbegin);
	void Init();
	State GetStates(const char *filename, vector <int> packages, vector <int> freeCores, int workflowSize);
	void OneStep(int periodNumber);
	vector <int> GetUOptsByStateNumber(int stateNum, int tbegin, ofstream &file, float & ef); //returns vector <number of uopts>
	int GetBusyCores(OneControl& control);
	int GetNextStateNumber(int stateNum, int uOptNum, ofstream & file);
	int GetStateNumber(vector <PackageState> &resultState, int stateNum);
	void SetFullState();
	void GetFullState(char * fname);
	bool CheckState (int stateNumber);
	void DirectBellman();
	float GetEfficiency(OneControl& control, int, int);
	PossibleControls CheckControls(int i, int);
	int GetTRealBegin(int l, float level, int periodBegin);
	float GetMaxEfficiency(){
		float eff = 0.0;
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
	}
	~Model(void);
};

