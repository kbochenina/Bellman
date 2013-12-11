#include "DataInfo.h"
#include <boost/tuple/tuple.hpp>
using namespace boost;

// (globalPackageNum, tBegin, vector<globalCoreNums>)
typedef vector <tuples::tuple<int,int,vector<int>>> Schedule;

#pragma once
class ScheduleToXML
{
	// model data
	DataInfo& data;
	string resFileName;
	string xmlBaseName;
public:
	ScheduleToXML(DataInfo &d): data(d) {}
	// create an XML file based on current data state (especially fixed intervals)
	// add packages from currentSchedule
	void CreateXML(Schedule currentSchedule);
	~ScheduleToXML(void);
};

