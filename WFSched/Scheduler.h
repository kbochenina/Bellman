#include "ModelData.h"
#include "ScheduleToXML.h"
#include <memory>

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
	// pointer to xml writer
	unique_ptr <ScheduleToXML> xmlWriter;
	// efficiency of fullSchedule
	double maxEff;
	// numbers of already scheduled WFs
	vector<int> scheduledWFs;
	// OPERATIONS
	// classical staged scheme
	void StagedScheme(int initWfNum);
	// staged scheme with known WF order
	void StagedScheme(vector <int>& wfOrder);
	// add to file info about schedule
	void PrintOneWFSched(ofstream & out, Schedule & sched, int wfNum);
public:
	Scheduler(ModelData& md);
	// fulfill the variable methodsSet according to nonexistent algorithm
	void SetSchedulingStrategy(int strategyNumber);
	// get schedule according to scheduleVariant parameter
	// 1 - classical staged scheme
	// 2 - staged scheme with known order
	void GetSchedule(int scheduleVariant);
	~Scheduler(void);
};

