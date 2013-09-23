#pragma once

extern int T, delta;

class Package
{
	int uid;
	bool isInit;
	vector <int> resourceTypes; // numbers of nodes where package can be executed
	vector <int> coreCounts; // numbers of cores where package can be executed 
	map <pair <int,int>, float> execTime; // ((1,2),15) means that package executes 15 seconds on 2 cores of restype 1
	vector <tuple<int,int,float>> packageStates; // (resType, cores, ready level)
public:
	Package(int u, vector <int> &r, vector <int> &c, map <pair <int,int>, float> &e);
	vector <int>& GetResourcesTypes() {return resourceTypes;}
	vector <int>& GetCoreCounts() {return coreCounts;}
	int GetResourcesTypesCount(){return resourceTypes.size();}
	float GetExecTime(int type, int cores){ return execTime[std::make_pair(type,cores)];}
	std::vector<int> GetCoresCount(){return coreCounts;}
	void SetIsInit(bool i) {isInit = i;}
	void SetPackageStates();
	int GetStatesCount() {return packageStates.size();}
	float GetLevel(int stateNum);
	int GetCore(int stateNum);
	int GetType(int stateNum);
	void GetResourceTypes(vector<int>&r) {r = resourceTypes;}
	void GetCoreCounts(vector<int>&c) {c = coreCounts;}
	void PrintState (ofstream & f, int &state);
	int GetNextStateNum(int currentStateNum, int controlType, int controlCore);
	~Package(void);
};

