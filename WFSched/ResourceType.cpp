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
