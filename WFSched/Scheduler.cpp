#include "StdAfx.h"
#include "Scheduler.h"
#include "UserException.h"
#include "SchedulingFactory.h"
#include "ScheduleToXML.h"
// in StagedScheme()
#include <time.h>
// StagedScheme() : to_string()
#include <string>
#include <iostream>
// StagedScheme() : ofstream
#include <fstream>

using namespace std;

Scheduler::Scheduler( ModelData& md ): data(md.GetData())
{
	methodsSet.resize(data.GetWFCount());
	maxEff = 0.0;
	xmlWriter = unique_ptr<ScheduleToXML>(new ScheduleToXML(data));
}


Scheduler::~Scheduler(void)
{
}

void Scheduler::SetSchedulingStrategy(int strategyNumber)
{
	switch (strategyNumber)
	{
		// only Bellman
	case 1: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 1;
		break;
		// only Greedy
	case 2: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 2;
		break;
		// mixed
	}
}

// implements staging scheme for finding the schedule for WFs set
// <PRE> 0 <= firstWfNum < data.workflows.size()
void Scheduler::StagedScheme(int firstWfNum){
	cout << "StagedScheme(int) was called\n";
	try{
		if (firstWfNum < 0 || firstWfNum > data.GetWFCount()) 
			throw UserException("Scheduler::StagedScheme(int) error. Wrong init workflow number");
		// creating XML with init time windows
		xmlWriter->SetXMLBaseName("Init_");
		Schedule oneWFsched;
		xmlWriter->CreateXML(oneWFsched);
		// ??!! think about it !
		xmlWriter->SetXMLBaseName("DP_");
		double stagedT = clock();

		string resFileName = "staged_scheme_" + to_string(firstWfNum) + ".txt";
		ofstream res(resFileName);
		if (res.fail()) 
			throw UserException("Scheduler::StagedScheme(int) error. Unable to create res file");
		res << "Stage 1, workflow # " << firstWfNum << endl;
		cout << "Stage 1, workflow # " << firstWfNum << endl;
		
		vector <double> eff;
		// applying settings of scheduling method for initial WF
		unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[firstWfNum], firstWfNum);
		method->printInfo();
		// getting schedule for first WF
		double oneStepStart = clock();
		eff.push_back(method->GetWFSchedule(oneWFsched));
		cout << "Elapsed time: " << (clock()-oneStepStart)/1000.0 << " sec" << endl;
		scheduledWFs.push_back(firstWfNum);	
		// write result to XML
		xmlWriter->CreateXML(oneWFsched);
		// write result to res file
		PrintOneWFSched(res, oneWFsched, firstWfNum);
		data.FixBusyIntervals();
		
	/*	ReadData(firstWfNum);
		int currentWfNum = firstWfNum;
		directBellman = false;
		BackBellmanProcedure();
		directBellman = true;
		eff.push_back(DirectBellman(firstWfNum));
		allStagesCores = stagesCores;
		BellmanToXML(true);
		//std::system("pause");
		stagesCores.clear();
		FixNewBusyIntervals();
		BellmanToXML(true);
		//std::system("pause");
		states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();*/
		
	/*	
		vector <vector <busyIntervals>> bestBusyIntervals;
		vector <tuples::tuple<int,int,vector<int>>> bestStagesCores;
		while (scheduledWFs.size() != Workflows.size()){
			cout << "stage " << scheduledWFs.size() + 1 << endl;
			double maxEff = 0.0;
			int bestWfNum = -1;
			for (vector<Workflow*>::size_type i = 0; i < Workflows.size(); i++){
				if (find(scheduledWFs.begin(), scheduledWFs.end(), i)== scheduledWFs.end()){
					currentWfNum = i;
					double s = clock();
					cout << "currentWfNum = " << currentWfNum << " ";
					ReadData(i);
					directBellman = false;
					BackBellmanProcedure();
					directBellman = true;
					double currentEff = DirectBellman(i);
					cout << "elapsed time: " << (clock()-s)/1000.0 << " sec" << endl;
					if (maxEff < currentEff){
						maxEff = currentEff;
						bestWfNum = i;
						bestStagesCores = stagesCores;
						bestBusyIntervals.clear();
						GetBestBusyIntervals(bestBusyIntervals);
					}
					states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
				}
			}
			int diapBegin = allStagesCores.size();
			copy(bestStagesCores.begin(), bestStagesCores.end(),back_inserter(allStagesCores));
			scheduledWFs.push_back(bestWfNum);
			usedNums = scheduledWFs;
			stagesCores = bestStagesCores;
			currentWfNum = bestWfNum;
			eff.push_back(maxEff);
			res << "WF " << bestWfNum << endl;
			for (vector <tuples::tuple<int,int,vector<int>>>::iterator it = bestStagesCores.begin(); it!= bestStagesCores.end(); it++){
				res << "(" << it->get<0>() << " " << it->get<1>() << " (";
				for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
					res << *it2 << " ";
				res << "))";
			}
			res << endl;
			BellmanToXML(true);
			//std::system("pause");
			stagesCores.clear();
			SetBestBusyIntervals(bestBusyIntervals);
			FixNewBusyIntervals();
			BellmanToXML(true);
			//std::system("pause");
		}
		usedNums = scheduledWFs;
		SetFirstBusyIntervals();
		stagesCores = allStagesCores;
		BellmanToXML(false);
		
		res << "Attempt " << firstWfNum << endl << "Workflow order: " ;
		for (vector<int>::size_type i = 0; i < usedNums.size(); i++){
			res << usedNums[i] << " ";
		}
		res << endl << "Efficiencies: " ;
		double sum = 0.0;
		for (vector<int>::size_type i = 0; i < eff.size(); i++){
			sum+=eff[i];
			res << eff[i] << " ";
		}
		res << endl << "Max eff: " << sum << endl << endl;
		SetFirstBusyIntervals();
		states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
		res.close();
		cout << "Max eff: " << sum << endl;
		cout << "Elapsed time: " << (clock()-stagedT)/1000.0 << " sec" << endl;*/
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
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

// add to file info about schedule
void Scheduler::PrintOneWFSched(ofstream & res, Schedule & sched, int wfNum){
	res << "WF " << wfNum << endl;
	for (Schedule::iterator it = sched.begin(); it!= sched.end(); it++){
		res << "(" << it->get<0>() << " " << it->get<1>() << " (";
		for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
			res << *it2 << " ";
		res << "))";
	}
	res << endl;
}