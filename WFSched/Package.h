#include <vector>
#include <map>
using namespace std;

#pragma once
class Package
{
	// package local uid - from 1
	int uid;
	// supported resource types
	vector <int> resTypes;
	// supported core counts
	vector <int> coreCounts;
	// exec times for pairs (resType, coreCount)
	map<pair<int,int>, double> execTimes;
public:
	Package(int u, vector<int>r, vector<int>c, map<pair<int,int>, double> e) {uid = u; resTypes = r; coreCounts = c; execTimes = e;}
	// getting execTime for choosed type and core
	double GetExecTime(int type, int cores);
	~Package(void);
};

