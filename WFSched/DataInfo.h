#include "Workflow.h"
#include "ResourceType.h"
#include "ModelingContext.h"
#include "memory.h"

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
	// getter 
	const vector<Workflow> & Workflows() const { return workflows; }
	const vector<ResourceType> & Resources() const { return resources; }
	const Workflow& Workflows(int wfNum) const {return workflows[wfNum]; }
	const ResourceType& Resources(int resNum) const {return resources[resNum]; }
	const vector<pair<int,int>> & TypesCores() const { return typesCores; }
	const pair<int,int>& TypesCores(int index) const {return typesCores[index]; }
	int GetDelta() {return context.GetDelta();}
	// get initial core index of resource type
	int GetInitResourceTypeIndex(int type);
	// do the same
	int GetTypeCoreIndex(const pair<int,int>& typeCore);
	~DataInfo(void);
};

