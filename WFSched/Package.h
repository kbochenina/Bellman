#include <vector>
#include <map>
using namespace std;

typedef map<pair<int,int>, double> times;

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
	times execTimes;
public:
	Package(int u, vector<int>r, vector<int>c, times e) {uid = u; resTypes = r; coreCounts = c; execTimes = e;}
	// getting execTime for choosed type and core
	double GetExecTime(int type, int cores);
	int GetResTypesCount () const {return resTypes.size();}
	int GetCoresCount () const {return coreCounts.size();}
	const vector <int>& GetResTypes() const {return resTypes;}
	const vector <int>& GetCoreCounts() const {return coreCounts;}
	const times& GetExecTimes() const {return execTimes;}
	~Package(void);
};

