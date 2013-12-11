#include "ScheduleToXML.h"
#include "Efficiency.h"
#pragma once

// provides finding schedule for one WF
class SchedulingMethod
{
	// scheduling method uid
	int uid;
protected:
	// reference on model data
	DataInfo& data;
	// current wfNum
	int wfNum;
	// efficiency finder
	Efficiency eff;
public:
	SchedulingMethod(DataInfo &d, int u, int w) : data(d), uid(u), wfNum(w) {}
	// get schedule for current method and WF
	// return value is efficiency
	// resulting schedule is stored in out parameter
	double GetWFSchedule(Schedule &out);
	~SchedulingMethod(void);
};

