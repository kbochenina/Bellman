#include "ScheduleToXML.h"
#include "Efficiency.h"
#include <memory>
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
	unique_ptr<Efficiency> eff;
public:
	SchedulingMethod(DataInfo &d, int u, int w) : data(d), uid(u), wfNum(w) {
		eff = unique_ptr<Efficiency> ( new Efficiency(2.00 / data.GetFullCoresCount()) );
	}
	// get schedule for current method and WF
	// return value is efficiency
	// resulting schedule is stored in out parameter
	virtual double GetWFSchedule(Schedule &out) = 0;
	// print info
	virtual void printInfo() = 0;
	~SchedulingMethod(void);
};

