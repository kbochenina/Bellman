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
	vector <vector<int>> packagesStates; // (stateNum for P1, stateNum for P2, ...)
	vector <ResourceType*>& _refResources; // for checking states correct by full core count
	vector <pair<unsigned short, unsigned short>> &_reftypesCores; // typesCores from Model for PrintControl() function
	float elapsedTime; // need to terminate function execution
	float maxPossibleTime;
	float beginTime, prevBeginTime;
	vector <vector<vector<int>>> controls; // index in typesCores, -1 means (0,0)
	vector<vector<int>> oneStateControls;
	vector <vector<vector<int>>> nextPackageStateNumbers; // for each state:  for each control - packages next state numbers
	vector <vector<int>> nextStateNumbers;

	public:
	Workflow (std::vector <Package*>, std::vector <std::vector <int>>, int, vector <ResourceType*>&, vector <pair<unsigned short, unsigned short>> &);
	std::vector <Package*> GetPackages() {return packages;} // is useful??
	int GetPackageCount(){return packages.size();}
	int GetExecTime(int pNum, int type, int cores) {return packages[pNum]->GetExecTime(type, cores);}
	std::vector<int> GetCoresCount(int pNum) {return packages[pNum]->GetCoresCount();}
	// pNum begins from ZERO
	bool IsPackageInit(int pNum);
	std::vector<int> GetPackageTypes(int pNum) {return packages[pNum]->GetResourcesTypes();}
	bool IsDepends(int one, int two);
	void GetDependency(int pNum, std::vector <int>& res );
	void SetIsPackageInit();
	void SetPackagesStates();
	void SetFullPackagesStates(int initPackage); // set Workflow::packagesStates
	bool Check (const int& state, const vector <int> & otherStates, const int & currentPackage); // check if state is compatible with other states
	void PrintPackagesStates();
	int GetStatesCount() {return packagesStates.size();}
	void GetControls (vector<int>& newState, int);
	bool CheckCores(pair<int,int> &, vector<int>&);
	void SetNextStateNumbers();
	void PrintControls();
	void CheckForReadiness(vector <int>&nextState); // if all packages-predecessors are ready, state must be set to (0,0)
};