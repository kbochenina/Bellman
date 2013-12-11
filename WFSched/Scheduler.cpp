#include "StdAfx.h"
#include "Scheduler.h"
#include <iostream>

using namespace std;

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
				methodsSet[i] = 1;
		break;
		// only Greedy
	case 1: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 2;
		break;
		// mixed
	case 2:
		break;
	}
}

void Scheduler::StagedScheme(int initWfNum){
	cout << "StagedScheme(int) was called\n";
}

void Scheduler::StagedScheme(vector<int>& order){
	cout << "StagedScheme(vector<int>&) was called\n";
}

void Scheduler::GetSchedule(int scheduleVariant){
	switch (scheduleVariant)
	{
	case 1:
		StagedScheme(0);
		break;
	case 2:
	{
		vector <int> order;
		for (int i = 0; i < data.GetWFCount(); i++)
			order.push_back(i);
		StagedScheme(order);
		break;
	}
	default:
		break;
	}
}