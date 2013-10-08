#pragma once

extern int T, delta;

class Package
{
	int uid;
	bool isInit;
	vector <int> resourceTypes; // numbers of nodes where package can be executed
	vector <int> coreCounts; // numbers of cores where package can be executed 
	map <pair <int,int>, double> execTime; // ((1,2),15) means that package executes 15 seconds on 2 cores of restype 1
	vector <tuple<int,int,double>> packageStates; // (resType, cores, ready level)
public:
	Package(int u, vector <int> &r, vector <int> &c, map <pair <int,int>, double> &e);
	vector <int>& GetResourcesTypes() {return resourceTypes;}
	vector <int>& GetCoreCounts() {return coreCounts;}
	int GetResourcesTypesCount(){return resourceTypes.size();}
	double GetExecTime(int type, int cores);
	double GetExecTime(const unsigned int & stateNum);
	double GetPartialExecTime(const unsigned int & stateNum);
	std::vector<int> GetCoresCount(){return coreCounts;}
	void SetIsInit(bool i) {isInit = i;}
	void SetPackageStates();
	int GetStatesCount() {return packageStates.size();}
	double GetLevel(unsigned int stateNum);
	int GetCore(unsigned int stateNum);
	int GetType(unsigned int stateNum);
	void GetResourceTypes(vector<int>&r) {r = resourceTypes;}
	void GetCoreCounts(vector<int>&c) {c = coreCounts;}
	void PrintState (ofstream & f, const int &state);
	int GetNextStateNum(unsigned int currentStateNum, int controlType, int controlCore);
	void PrintExecTime(ofstream &f);
	void GetExecTime(map <pair <int,int>, double> & out) {out = execTime;}
	~Package(void);
};

