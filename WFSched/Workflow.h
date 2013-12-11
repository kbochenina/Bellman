#include "Package.h"

#pragma once
class Workflow
{
	// workflow number - from 1
	int uid;
	// packages of WF
	vector <Package> packages;
	// connectivity matrix
	vector<vector<int>> matrix;
public:
	// return package count
	inline int GetPackageCount() { return packages.size(); } 
	// return execTime of package pNum on resource with type resType and coreCount
	double GetExecTime(int pNum, int resType, int coreCount);
	// return vector with possible cores count for package pNum
	void GetCoresCount(int pNum, vector<int>&out);
	// return vector with possible resource types for package pNum
	void GetResTypes(int pNum, vector<int>&out);
	// return true if package pNum is init
	bool IsPackageInit(int pNum);
	// return true if first depends on second
	bool IsDepends(int one, int two);
	// return vector with packages from which pNum depends (?)
	void GetDependency(int pNum, vector<int>& out);
	Workflow(int u, vector <Package> p, vector<vector<int>> m) {uid = u; packages = p; matrix = m;}
	Workflow(){ uid = 0; }
	~Workflow(void);
};

