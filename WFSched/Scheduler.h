#include "ModelData.h"
#include "ScheduleToXML.h"

#pragma once
// SCHEDULING METHODS: 1 - Bellman, 2 - Greedy

class Scheduler
{
	// ATTRIBUTES
	// codes of scheduling methods for all WFs
	vector<int> methodsSet;
	// schedule for all WFs after optimizing
	Schedule fullSchedule;
	// reference on dataInfo
	DataInfo &data;
	// efficiency of fullSchedule
	double maxEff;
	// numbers of already scheduled WFs
	vector<int> scheduledWFs;
	// OPERATIONS
	// classical staged scheme
	void StagedScheme(int initWfNum);
	// staged scheme with known WF order
	void StagedScheme(vector <int>& wfOrder);
public:
	Scheduler(ModelData& md);
	// fulfill the variable methodsSet according to nonexistent algorithm
	void SetSchedulingStrategy(int strategyNumber);
	// get schedule according to scheduleVariant parameter
	void GetSchedule(int scheduleVariant);
	~Scheduler(void);
};

