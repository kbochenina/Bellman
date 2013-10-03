#pragma once
#include <vector>
#include "Resource.h"

extern int T, delta, stages;
extern vector<int> stageBorders;
extern bool canExecuteOnDiffResources;

class ResourceType
{
	int type;
	int numCoresPerOneRes;
	double perf;
	unsigned short initVal, lastVal; // first and last index in typesCores
	std::vector <int> forcedBricks;
	std::vector<std::vector <int>> allCoresFreeTimes; // (stageNum, (core1TimeEnd, core2TimeEnd...,coreN))
	std::vector <Resource> resources;
	int GetRightBorderForOneCore(const unsigned int &core, const unsigned int &stage);
	int GetLeftBorderForOneCore(const unsigned int &core, const unsigned int &stage);
	int GetResourceIndex(int core);
	bool IsDifferentResources(int core1, int core2);
public:
	
	ResourceType(int t, std::vector<Resource> r, double p): type(t), resources(r), perf(p){
		if (r.size()!=0) numCoresPerOneRes = r[0].GetCoresCount();
	};
	~ResourceType(void);
	int GetResourceCount(){return resources.size();}
	Resource& operator[](const int index){return resources[index];}
	bool Check(const vector<pair<double,unsigned int>>& timeCores, const int &stage,  
		vector<vector<int>>& oneTypeCoreNums, bool isCheckedForState, vector <int>& addForbiddenCoreNums);
	void SetInitLastVals(unsigned short i, unsigned short l) {initVal = i; lastVal = l;}
	unsigned short GetInitVal(){return initVal;}
	void SetFreeTimeEnds(); // if we want to set default free time ends;
	void SetFreeTimeEnds(const std::vector<int>&);
	void SetForcedBricks(std::vector <int> fB) { forcedBricks = fB;}
	void CorrectBusyIntervals(const std::vector <int> &);
	int GetOneResCoresCount(){return numCoresPerOneRes;}
	int GetCoresCount();
	double GetPerf(){return perf;}
	bool IsPossible(int execTime, int stage, int numCores);
	
	int GetFinalStage(const std::vector<int> &execTimes, const std::vector<int> &stages, const std::vector<int>&numCores, 
		const std::vector <int>& addForcedBricks, std::vector <unsigned int>&, std::vector <unsigned int>&, std::vector<std::vector<int>>&); // if return 0 then no possible combinations
	void SetChildForcedBricks(){for (unsigned int i = 0; i < resources.size(); i++) resources[i].SetForcedBricks();}
	std::vector<int> GetForcedNumbers(const int &execTime, const int &stageBegin, int numCores = 1);
	void AddDiaps(int stageBegin, int stageCount, vector<int>& cores);
	void SetInitBusyIntervals();
};

