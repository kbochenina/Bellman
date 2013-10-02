#pragma once


#include "Workflow.h"
#include "ResourceType.h"
#include "UserException.h"


typedef pair <int, double> StateInformation; // (number of uopt, max eff on this state)
typedef vector <StateInformation> StageInformation;
typedef vector <StageInformation> BackBellmanInfo;
typedef vector<vector<pair<double, unsigned int>>> timeCore;

extern int T, delta, stages;
extern vector<int> stageBorders;
extern bool canExecuteOnDiffResources; // if true one package can execute on some cores of different resources of one type

class Model
{
	// fields
	vector <Workflow*> Workflows;
	std::vector <ResourceType*> Resources;
	vector <tuple<int,int,vector<int>>> stagesCores; // (package,stage, core1, core2,...) for package1, (stage, core1, core2, ...) for package 2...
	vector <tuple<int,int,vector<int>>> allStagesCores; // 
	std::vector <int> forcedBricks;
	double koeff;
	vector <pair<unsigned short, unsigned short>> typesCores;
	int fullCoresCount;
	string resFileName;
	int currentWfNum;
	vector <int> usedNums;
	// for each state: vector of controls. Each package control can be -1 (if no resources used)
	// or index in typesCores
	vector <vector<vector<int>>> controls; 
	vector <vector<int>> nextStateNumbers;
	vector <vector<int>> states; // (stateNum for P1, stateNum for P2, ...)
	vector <pair<vector<int>,vector<int>>>fullUsedNums;
	BackBellmanInfo FullInfo;
	string xmlBaseName;
	// methods
	void InitResources(string);
	void InitWorkflows(string);
	bool CheckState (const unsigned int state, const unsigned int stage,timeCore& typeCoresPerType, vector <vector<int>>& packagesCoreNums);
	bool CheckControl(const unsigned int &state, const unsigned int &control, const unsigned int &stage, timeCore&, 
		bool isUsedNumsNeeded, vector<vector<int>> &stageUsedNums, bool debugFlag);
	void SetForcedBricks();
	double efficiencyFunction(double x) { return (koeff*(1-x)); }
	double EfficiencyByPeriod(int busyCores, int t1, int t2){
		return ( busyCores * (double)(t2-t1)/(double)T * (efficiencyFunction((double)t1/(double)T) + efficiencyFunction((double)t2/(double)T)) / 2  );
	}
	int GetStatesCount() {return states.size();}
	double GetEfficiency(const int & stage, const timeCore& currentTC);
	void BellmanToXML(bool isOne);
	void MetaXMLInfo(ofstream &f);
	void BusyToXML(ofstream &f);
	void StagesCoresToXML(ofstream&f);
	void ReadData(int wfNum);
	void GetStageInformation(int stage);
	double DirectBellman(int wfNum);
	void BackBellmanProcedure();
	void AddDiaps(int beginIndex, int wfNum);
	int GetResourceType(int number);
	int GetResourceTypeBeginIndex(int type);
	void Model::StagesCoresToXML(ofstream&f, int currentWfNum);
public:
	Model(){}
	void StagedScheme(int firstWFNum); //  firstWFNum from ZERO
	void Init (string resFile, string wfFile, string settingsFile, string xmlFile);
	void InitSettings(string);
	double Greedy(int uopt, double currentEff);
	
	~Model(void);
};

