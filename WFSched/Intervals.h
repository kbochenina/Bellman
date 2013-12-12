#include <vector>
#include <map>
#include "ModelingContext.h"
using namespace std;

#pragma once
typedef map<int,vector<pair<int,int>>> BusyIntervals;
class Intervals
{
	// see for detailed description UML document IntervalsStates
	// init intervals
	vector <BusyIntervals> init;
	// intervals after fixing the borders
	vector <BusyIntervals> fixed;
	// current windows
	vector <BusyIntervals> current;
	// core count
	int numCores;
	ModelingContext context;
	// for each stage, for each core - free time on this stage
	vector <vector<int>> allCoresFreeTimes;
	// OPERATIONS
	// correct busy intervals (set appropriate to stageBorders)
	void Correct();
	// set allCoresFreeTimes according to "current"
	void SetFreeTime();
public:
	Intervals(){ numCores = 0; }
	void SetData(vector<BusyIntervals> i, ModelingContext& context);
	inline void Reset() { current = fixed;}
	inline void Fix() { fixed = current; }
	inline void SetInit() { fixed = current = init; }
	// add busy intervals [tBegin; (tBegin + execTime) round to highest stage border] to cores in coreNumbers
	void AddDiaps(vector <int> coreNumbers, int tBegin, double execTime);
	vector <BusyIntervals> GetCurrentIntervals() { return current; }
	vector <BusyIntervals> GetFixedIntervals() { return fixed; }
	void SetCurrentIntervals(vector <BusyIntervals> & in) { current = in; } 
	~Intervals(void);
};

