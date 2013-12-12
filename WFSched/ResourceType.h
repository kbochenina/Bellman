#include "Intervals.h"
#include "ModelingContext.h"


#pragma once

typedef vector<pair<double,int>> TimeCore;

class ResourceType
{
	// resource number - from 1
	int uid;
	// cores count per one resource of this type
	int numCoresPerOneRes;
	// resource count
	int resCount;
	// performance (GFlop/s)
	double perf;
	// busy time windows
	Intervals windows;
	// if one package can execute of different resources, variable is true
	bool canExecuteOnDiffResources;
public:
	ResourceType(int u, int nC, int rC, double p, vector<BusyIntervals> i, bool flag, ModelingContext& context);
	// check if elements of vector timeCores (execTime, coreCount) can be placed in the moment tbegin
	// to a resources of this type (also according to checkType). Possible placement will be stored
	// to out variable. If placement is impossible, function will return false
	bool Check(const vector<TimeCore>& timeCores, int tBegin, bool checkType, vector<vector<int>>&out);
	inline int GetCoresCount() {return numCoresPerOneRes * resCount; }
	inline double GetPerf() {return perf;}
	void FixBusyIntervals();
	void ResetBusyIntervals();
	void SetInitBusyIntervals();
	~ResourceType(void);
};

