#include "StdAfx.h"
#include "Package.h"


Package::Package(int u, vector <int> &r, vector <int> &c, map <pair <int,int>, int> &e)
{
	uid = u;
	resourceTypes = r;
	coreCounts = c;
	execTime = e;
}


Package::~Package(void)
{
}
