#include "StdAfx.h"
#include "Package.h"


Package::Package(int u, vector <int> &n, vector <int> &c, map <pair <int,int>, int> &e)
{
	uid = u;
	nodeNumbers = n;
	coreCounts = c;
	execTime = e;
}


Package::~Package(void)
{
}
