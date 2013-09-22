#pragma once


#include "Workflow.h"
#include "ResourceType.h"


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

extern int T, delta, stages;
extern vector<int> stageBorders;

class Model
{
	// fields
	vector <Workflow*> Workflows;
	std::vector <ResourceType*> Resources;
	std::vector <int> forcedBricks;
	float koeff;
	vector <pair<unsigned short, unsigned short>> typesCores;
	friend class Workflow;
	// methods
	void InitResources(string);
	void InitWorkflows(string);
	void Model::SetForcedBricks();


	vector <pair<int,int>> diff;
	float maxEfficiency ;
	vector <State> States;
	vector <int> repeatCounts;
	State FullState;
	Control FullControl;
	BackBellmanInfo FullInfo;
	StageInformation StageTable;
	vector <int> pControlNumbers;
	NextStates FullNextState;
	
	
	
public:
	Model(){}
	void Init (string resFile, string wfFile);
	float efficiencyFunction(float x) { return (koeff*(1-x)); }
	float EfficiencyByPeriod(int busyCores, int t1, int t2){
		return ( busyCores * (float)(t2-t1)/(float)T * (efficiencyFunction((float)t1/(float)T) + efficiencyFunction((float)t2/(float)T)) / 2  );
	}
	// debug and time measurement functions
	int GetStatesCount() {return Workflows[0]->GetStatesCount();}
	
	
	float Greedy(int uopt, float currentEff);
	void GetStageInformation(int stage);
	void CheckForIllegalCoreNumber(vector<vector <OnePackageControl>> &fullPossibleControls);
	PossibleControls GetStateControls(int stateNumber, ofstream& file);
	void GetOneStageControls(int tbegin);
	State GetStates(string filename, int initPackage, int wfNum);
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
	/*float GetMaxEfficiency(){
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
	}*/
	~Model(void);
};

