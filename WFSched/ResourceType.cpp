#include "StdAfx.h"
#include "ResourceType.h"


ResourceType::ResourceType(int u, int nC, int rC, double p, vector<BusyIntervals> bi, bool flag, ModelingContext& context)
{
	uid = u;
	numCoresPerOneRes = nC;
	resCount = rC;
	perf = p;
	windows.SetData(bi, context);
	canExecuteOnDiffResources = flag;
}


ResourceType::~ResourceType(void)
{
}

// fix busy intervals after scheduling
void ResourceType::FixBusyIntervals(){
	windows.Fix();
}

// set current busy intervals to fixed
void ResourceType::ResetBusyIntervals(){
	windows.Reset();
}

// set current and fixed busy intervals to init
void ResourceType::SetInitBusyIntervals(){
	windows.SetInit();
}


