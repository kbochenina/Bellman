#include "Workflow.h"
#include "ResourceType.h"
#include "ModelingContext.h"

using namespace std;

#pragma once
class DataInfo
{
	// friend classes
//	friend class Scheduler;
	friend class ScheduleToXML;
	// ATTRIBUTES
	// modeling time characteristics
	ModelingContext context;
	// vector of model workflows
	vector <Workflow> workflows;
	// vector of model resource types
	vector <ResourceType> resources;
	// metaWF is WF composed of all WFs
	Workflow metaWF;
	// variants of controls: (typeIndex1, coreCount1), (typeIndex1, coreCount2),..., (typeIndexN, coreCountN)
	vector <pair<int,int>> typesCores;
	// sum of resource types core count
	int fullCoresCount;
	//OPERATIONS
	// init data placement settings
	void Init(string fName);
	// init resources
	void InitResources(string fName, bool canExecuteOnDiffResources);
	// init workflows
	void InitWorkflows(string fName);
	
public:
	DataInfo(){}
	DataInfo(string fSettings);
	// create a metaWF
	void CreateMetaWF();
	// get WF count
	inline int GetWFCount() {return workflows.size();}
	inline int GetResourceCount() {return resources.size(); }
	// get full cores count
	inline int GetFullCoresCount() {return fullCoresCount;}
	int GetResourceType (int coreNumber);
	void FixBusyIntervals();
	void ResetBusyIntervals();
	void SetInitBusyIntervals();
	void GetCurrentIntervals(vector<vector<BusyIntervals>> &storedIntervals);
	void SetCurrentIntervals(vector<vector<BusyIntervals>> &storedIntervals);
	~DataInfo(void);
};

