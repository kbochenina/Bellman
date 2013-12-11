#include "StdAfx.h"
#include "Scheduler.h"



Scheduler::Scheduler( ModelData& md ): data(md.GetData())
{
	methodsSet.resize(data.GetWFCount());
	maxEff = 0.0;
}


Scheduler::~Scheduler(void)
{
}

void Scheduler::SetSchedulingStrategy(int strategyNumber)
{
	switch (strategyNumber)
	{
		// only Bellman
	case 0: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 0;
		break;
		// only Greedy()
	case 1: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 1;
		break;
		// mixed
	case 2:
		break;
	}
}
