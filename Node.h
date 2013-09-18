#pragma once
#define MAX_TIME 100000
class Node
{
	int coreCount; // number of cores
	int priority; // priority of node
	multimap <int,pair<int,int>> busyIntervals; // (coreCount, tBegin, tEnd) for busy intervals
public:
	Node(int c, int p, multimap <int,pair<int,int>> &bi);
	int GetFreeCores(int tbegin);
	int GetCoreCount(){return coreCount;}
	// not working when busy intervals for one number of cores is more than 1
	int GetNextBusy(int core, int t){
		int result = 0;
		multimap <int,pair<int,int>>::iterator it = busyIntervals.find(core);
		if (it!=busyIntervals.end()){
			if (t < (*it).second.first) return (*it).second.first;
		}
		return 0;
	}
	multimap <int,pair<int,int>> GetBusyIntervals(){return busyIntervals;}
	int GetFreeEnd(int core, int t){
		// for example: free time for 3 cores will end, when 4 + 1 - 3 = 2 and more cores are busy 
		int mustBusyMinCoreCount = coreCount + 1- core;
		int freeTimeEnd = MAX_TIME;
		for (int i = mustBusyMinCoreCount; i <= coreCount; i++){
			multimap <int,pair<int,int>>::iterator it;
			int timeCoreEnd = 0;
			// for all intervals for this core number
			for (it = busyIntervals.lower_bound(i); it!= busyIntervals.upper_bound(i); it++){
				// if we have one range for this count
				int rangeBegin = (*it).second.first;
				if (t < rangeBegin){
					timeCoreEnd = rangeBegin;
					break;
				}
				else if (t < (*it).second.second) {
					return 0;
				}
			}
			if (timeCoreEnd < freeTimeEnd && timeCoreEnd!=0) freeTimeEnd = timeCoreEnd;
		}
		return freeTimeEnd;
	}
	~Node(void);
};

