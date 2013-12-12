#include "SchedulingMethod.h"
#include "BellmanScheme.h"
#include "DataInfo.h"
#include <memory>
#pragma once
class SchedulingFactory
{
public:
	static unique_ptr<SchedulingMethod> GetMethod(DataInfo &d,int uid, int wfNum);
	SchedulingFactory();
	~SchedulingFactory(void);
};

