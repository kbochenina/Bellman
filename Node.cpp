#include "StdAfx.h"
#include "Node.h"
#include <vector>

Node::Node(int c, int p, multimap <int,pair<int,int>> &fi)
{
	coreCount = c;
	priority = p;
	busyIntervals = fi;
}


int Node::GetFreeCores(int tbegin){
	for (multimap<int,pair<int,int>>::iterator it = busyIntervals.begin(); it!= busyIntervals.end(); it++){
		// (12000; 15000), tbegin = 13000
		if (tbegin >= (*it).second.first && tbegin <= (*it).second.second) return coreCount- (*it).first;
	}
	return coreCount;
}

Node::~Node(void)
{
}
