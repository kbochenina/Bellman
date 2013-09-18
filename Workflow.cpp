#include "StdAfx.h"
#include "Workflow.h"


Workflow::Workflow(vector <int> &p, map <int, vector<int>> &l, map <int, vector<int>> &dl)
{
	packageNumbers = p;
	links = l;
	directlinks = dl;
}


Workflow::~Workflow(void)
{
}
