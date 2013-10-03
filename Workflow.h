#include "stdafx.h"
#include "Package.h"
#include "ResourceType.h"
#include <vector>
#include <map>

extern ofstream ex;

class Workflow{
	int wfNum;
	std::vector <Package*> packages;
	std::vector <std::vector <int>> connectMatrix;
	vector <ResourceType*>& _refResources; // for checking states correct by full core count
	vector <pair<unsigned short, unsigned short>> &_reftypesCores; // typesCores from Model for PrintControl() function
	double elapsedTime; // need to terminate function execution
	double maxPossibleTime;
	double beginTime, prevBeginTime;
	vector<vector<int>> oneStateControls;
	vector <vector<vector<int>>> nextPackageStateNumbers; // for each state:  for each control - packages next state numbers
	public:
	Workflow (std::vector <Package*>, std::vector <std::vector <int>>, int, vector <ResourceType*>&, vector <pair<unsigned short, unsigned short>> &);
	std::vector <Package*> GetPackages() {return packages;} // is useful??
	int GetPackageCount(){return packages.size();}
	double GetExecTime(int pNum, int type, int cores);
	std::vector<int> GetCoresCount(int pNum) {return packages[pNum]->GetCoresCount();}
	// pNum begins from ZERO
	bool IsPackageInit(int pNum);
	std::vector<int> GetPackageTypes(int pNum) {return packages[pNum]->GetResourcesTypes();}
	bool IsDepends(unsigned int one, unsigned int two);
	void GetDependency(int pNum, std::vector <int>& res );
	void SetIsPackageInit();
	void SetPackagesStates();
	void SetFullPackagesStates(int initPackage, vector <vector<int>>&, vector <vector<vector<int>>>&, vector <vector<int>>&); // set Workflow::packagesStates
	bool Check (const int& state, const vector <int> & otherStates, const int & currentPackage); // check if state is compatible with other states
	void PrintPackagesStates(vector <vector<int>>&);
	void GetControls (vector<int>& newState, unsigned int);
	bool CheckCores(pair<int,int> &, vector<int>&);
	void SetNextStateNumbers(vector <vector<int>>&, vector <vector<vector<int>>>&, vector <vector<int>>&);
	void PrintControls(vector <vector<int>> &, vector <vector<vector<int>>>&, vector <vector<int>>&);
	void CheckForReadiness(vector <int>&nextState); // if all packages-predecessors are ready, state must be set to (0,0)
	bool CheckState(int stateNumber, int stage);
	void SetTimesCoresForState(const vector<int>&state, vector<vector<pair<double, unsigned int>>>&coresPerType, 
		vector<vector<int>>& packagesIndexesPerType);
	void SetTimesCoresForControl(const vector<int>&state, const vector<int>&control, vector<vector<pair<double, unsigned int>>>&coresPerType,
		vector<vector<int>>& packagesIndexesPerType);
	double GetLevel(int pNum, int state);
	void PrintState(const vector <int>& state, ofstream &f);
	void PrintControl(const vector <int>& control, ofstream &f);
	void PrintExecTime();
	double GetExecTime(int pNum, int stateNum);
};