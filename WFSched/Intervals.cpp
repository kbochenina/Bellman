#include "StdAfx.h"
#include "Intervals.h"
#include "UserException.h"

void Intervals::SetData(vector<BusyIntervals> i, ModelingContext& c){ 
	init = i; 
	context = c;
	if (init.size() > 0) 
		// numCoresPerOneRes * numResources
		numCores = init[0].size() * init.size(); 
	else numCores = 0; 
	int stageCount = context.GetStages();
	// allCoresFreeTimes[stagecount * numcores]
	allCoresFreeTimes.resize(stageCount);
	for (int i = 0; i < stageCount; i++)
		allCoresFreeTimes[i].resize(numCores);
	Correct();
	SetFreeTime();
}

void Intervals::Correct(){
	const vector<int> &stageBorders = context.GetBorders();
	// for each resource intervals
	for (vector<BusyIntervals>::iterator resIt = init.begin(); resIt!= init.end(); resIt++){
		// for each core
		for (BusyIntervals::iterator it = resIt->begin(); it!= resIt->end(); it++){
			vector <int> toEraseNums;
			int newVal = 0; 
			for (vector<pair<int,int>>::iterator it2 = it->second.begin(); it2!=it->second.end(); it2++){
				// if second border of changed interval > than left border of next interval
				// we need to join it
				if (newVal > it2->first){
					// left border of joined interval is equal to left border of changed interval
					it2->first = (it2-1)->first;
					// after joining changed interval will be erased
					toEraseNums.push_back(distance(it->second.begin(), it2-1));
				}
				int bEnd = it2->second;
				for (unsigned int i = 0; i < stageBorders.size()-1; i++) {
					// if end of interval is among stageBorders[i] and stageBorders[i+1]
					// it will be equal to stageBorders[i+1]
					if (stageBorders[i] < bEnd && stageBorders[i+1] > bEnd) {
						newVal = it2->second = stageBorders[i+1];
					}
				}
			}
			// erasing unused intervals
			for (unsigned int i = 0; i < toEraseNums.size(); i++){
				it->second.erase(it->second.begin()+toEraseNums[i]);
			}
		}
	}
	SetInit();
}

// ((core1 freeTime, core2 FreeTime), (core 1 freeTime, core2 FreeTime)) - stage 
void Intervals::SetFreeTime(){
	int delta = context.GetDelta();
	int coreIndex = 0; //?
	// for each resource
	for (vector<BusyIntervals>::iterator resIt = current.begin(); resIt!= current.end(); resIt++){
		// for each core
		for (BusyIntervals::iterator it = resIt->begin(); it!= resIt->end(); it++){
			// if core has no busy windows, all its free times = delta
			if (it->second.size()==0) {
				for (int i = 0; i < context.GetStages(); i++)
					allCoresFreeTimes[i][coreIndex] = delta;
				coreIndex++;
				continue;
			}
			vector <pair<int, int>> intervals = it->second;
			int indexBusyInterval = 0;
			for (int i = 0; i < context.GetT(); i+=delta){
				int bBegin = intervals[indexBusyInterval].first;
				int bEnd = intervals[indexBusyInterval].second;
				// if right border are not equal to stage border
				// we should ceil it to highest stage border
				if (bEnd % delta!=0) {
					bEnd = (bEnd/delta + 1) * delta;
				}
				// debug part
				if (coreIndex > numCores - 1) 
					throw UserException("Resource::GetFreeTime(): wrong coreIndex");
				// if current stage is not busy by [bBegin; bEnd],
				// allCoresFreeTimes = delta
				if (i < bBegin && i+delta <= bBegin) 
					allCoresFreeTimes[i/delta][coreIndex] = delta;
				// if bBegin is in [i; i+delta]
				else {
					// set particle free time for current stage
					allCoresFreeTimes[i/delta][coreIndex] = bBegin - i;
					// move to next stage
					i+=delta;
					// bEnd is divisible by i
					while(i != bEnd) {
						// debug part
						if (i > context.GetT()) {
							/*vector <pair<int, int>>::iterator it = intervals.begin();
							for (; it!= intervals.end(); it++){
								cout << it->first << " " << it->second << endl;
							}
							cout << "bBegin = " << bBegin << endl;
							cout << "bEnd = " << bEnd << endl;*/
							throw UserException("Intervals::SetFreeTime(): out of range exception");
						}
						allCoresFreeTimes[i/delta][coreIndex] = 0;
						i+=delta;
					}

					indexBusyInterval++;
					// if there are no more intervals  
					if (indexBusyInterval == intervals.size())
						while (i < context.GetT()) {
							allCoresFreeTimes[i/delta][coreIndex] = delta;
							i+=delta;
						}
					// to cancel last increment of i (in first or second while)
					i-=delta;
				}
			}
			coreIndex++;
		}
	}
}

Intervals::~Intervals(void)
{
}
