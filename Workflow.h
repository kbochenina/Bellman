#include "stdafx.h"
#include "Package.h"
#include "ResourceType.h"
#include <vector>
#include <map>


class Workflow{
	std::vector <Package*> packages;
	std::vector <std::vector <int>> connectMatrix;
	public:
	Workflow (std::vector <Package*>, std::vector <std::vector <int>>);
	std::vector <Package*> GetPackages() {return packages;} // is useful??
	int GetPackageCount(){return packages.size();}
	int GetExecTime(int pNum, int type, int cores) {return packages[pNum]->GetExecTime(type, cores);}
	std::vector<int> GetCoresCount(int pNum) {return packages[pNum]->GetCoresCount();}
	// pNum begins from ZERO
	bool IsPackageInit(int pNum);
	std::vector<int> GetPackageTypes(int pNum) {return packages[pNum]->GetResourcesTypes();}
	bool IsDepends(int one, int two);
	void GetDependency(int pNum, std::vector <int>& res );
};