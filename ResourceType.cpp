#include "StdAfx.h"
#include "ResourceType.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
using namespace std;

bool SortPair(const std::pair <int, int> &p1, const std::pair <int, int> &p2)
{
	return p1.second > p2.second;
}

void ResourceType::CorrectBusyIntervals(const vector <int> &  stageBorders){
	for (unsigned int i = 0; i < resources.size(); i++)
		resources[i].CorrectBusyIntervals(stageBorders);
}

int ResourceType::GetCoresCount(){
	int coresCount = 0;
	for (unsigned int i = 0; i < resources.size(); i++)
		coresCount += resources[i].GetCoresCount();
	return coresCount;
}

bool ResourceType::IsPossible(int execTime, int stage, int numCores){
	vector <int> nearestBorders;
	for (unsigned int i = 0; i < resources.size(); i++){
		int near = resources[i].GetNearestBorder(numCores, stage);
		if (near!= 0) nearestBorders.push_back(near);
	}
	if (nearestBorders.size()==0) return false;
	vector<int>::iterator it = max_element(nearestBorders.begin(), nearestBorders.end());
	if (*it >= stage + execTime) return true;
	else if (*it == T) return true;
	else return false;
}

std::vector<int> ResourceType::GetForcedNumbers(const int &execTime, const int &stageBegin, int numCores){
	vector <int> forcedNumbers;
	for (unsigned int i = 0; i < resources.size(); i++){
		int near = resources[i].GetNearestBorder(numCores, stageBegin);
		if (near!=T){
			if (near== 0 || near < stageBegin + execTime) forcedNumbers.push_back(i);
		}
	}
	return forcedNumbers;
}

int ResourceType::GetFinalStage(const std::vector<int> &execTimes, const std::vector<int> &stages, 
								const std::vector<int>&numCores, const std::vector <int>& addForcedBricks, 
								vector<unsigned int>& control, vector <unsigned int>& nextStage, std::vector<std::vector<int>>&coreNumbers){
	int numResources = resources.size();
	if (addForcedBricks.size()!=0){
		try{
			if (addForcedBricks.size()!=numCoresPerOneRes*numResources) throw "ResourceType::GetFinalStage() error";
			vector <vector<unsigned int>> forcedBricksToResources;
			int addForcedIndex = 0;
			forcedBricksToResources.resize(numResources);
			for (int i = 0; i < numResources; i++){
				for (int j = 0; j < numCoresPerOneRes; j++)
					forcedBricksToResources[i].push_back(addForcedBricks[addForcedIndex++]);
				resources[i].AddForcedBricks(forcedBricksToResources[i]);
			}
		}
		catch (const string msg){
			cout << msg << endl;
			system("pause");
			exit(EXIT_FAILURE);
		}
	}
	int packageNum = execTimes.size();

	vector <pair<int,int>> length;
	for (int i = 0; i < packageNum; i++)
		length.push_back(make_pair(i,execTimes[i]));
	std::sort(length.begin(), length.end(), SortPair);
	
	int shift = T/delta - 1;
	for (int i = 0; i < numResources*numCoresPerOneRes; i++) {
		control.push_back(0);
		nextStage.push_back(0);
	}

	coreNumbers.resize(packageNum);

	int finalStage = 0;
	
	vector<vector<int>> unsortedBestCores;

	for (unsigned int i = 0; i < length.size(); i++){
		int currentPackageNum = length[i].first;
		int currentCoresNum = numCores[currentPackageNum];
		int execTime = execTimes[currentPackageNum], stagesCount = 0;
		stagesCount = execTime / delta + 1;
		int stageBegin = stages[currentPackageNum];
		int bestDistance = (T+delta)*currentCoresNum, currentDistance = 0, bestResource = 0;
		int stageEnd = (stageBegin + execTime) / delta;
	
		if (stageEnd > finalStage) finalStage = stageEnd;
		
		vector <int> bestCores;
		for (int j = 0; j < numResources; j++){
			vector <int> currentCores;
			currentCores.resize(currentCoresNum);
			currentDistance = resources[j].GetPlacement(stages[currentPackageNum],execTimes[currentPackageNum],
														currentCoresNum, currentCores);
			if (currentDistance < bestDistance && currentDistance!=-1){
				bestDistance = currentDistance;
				bestCores = currentCores;
				bestResource = j;

			}
		}

		if (bestDistance == (T+delta)*currentCoresNum) {
				for (int i = 0; i < numResources; i++) resources[i].SetForcedBricks();
				return -1;
		}

		vector <int>::iterator coreIt= bestCores.begin();
		for (; coreIt!= bestCores.end(); coreIt++){
			for (int i = stageBegin/delta; i < stageBegin/delta + stagesCount; i++){
				control[bestResource*numCoresPerOneRes + *coreIt] |= 1 << (shift - i);
			}
		}
		vector <unsigned int> oneResAddFBricks;
		for (int i = bestResource*numCoresPerOneRes; i < (bestResource+1)*numCoresPerOneRes; i++)
			oneResAddFBricks.push_back(control[i]);
		resources[bestResource].AddForcedBricks(oneResAddFBricks);

		for (unsigned int i = 0; i < bestCores.size(); i++) bestCores[i]+=bestResource*numCoresPerOneRes;

		unsortedBestCores.push_back(bestCores);
		//for (int j = 0; j < bestCores.size(); j++) forcedCores[bestResource].push_back(bestCores[i]);
		//controlInfo[bestResource].push_back(make_pair(currentPackageNum, bestCores));
	}
	
	
	vector <unsigned int> oneResStage;
	for (int i = 0; i < numResources; i++){
		oneResStage = resources[i].GetForcedBricks();
		for (int j = 0; j < numCoresPerOneRes; j++)
			nextStage[i*numCoresPerOneRes + j] = oneResStage[j]; 
	}

	for (unsigned int i = 0; i < length.size(); i++)
		coreNumbers[length[i].first] = unsortedBestCores[i];

	for (int i = 0; i < numResources; i++) resources[i].SetForcedBricks();
	
	return finalStage;

}

void ResourceType::SetFreeTimeEnds(){
	allCoresFreeTimes.clear();
	vector<vector <int>> resourceFreeTimes;
	resourceFreeTimes.resize(T/delta);
	allCoresFreeTimes.resize(T/delta);
	for (int i = 0; i < T/delta; i++) resourceFreeTimes[i].resize(numCoresPerOneRes);
	for (unsigned int i = 0; i < resources.size(); i++){
		resources[i].GetFreeTime(resourceFreeTimes);
		for  (unsigned int j = 0; j < resourceFreeTimes.size(); j++)
			copy(resourceFreeTimes[j].begin(), resourceFreeTimes[j].end(), back_inserter(allCoresFreeTimes[j]));
	}
}

// check!!!
void ResourceType::SetFreeTimeEnds(const vector <int>&addForcedBricks){
	for (unsigned int i = 0; i < addForcedBricks.size(); i++){
		int forcedBrick = addForcedBricks[i];
		int rowNum = (forcedBrick-1)% (T/delta);
		int columnNum = (forcedBrick-1) / (T/delta);
		allCoresFreeTimes[rowNum][columnNum] = 0;
	}

}

int ResourceType::GetRightBorderForOneCore(const unsigned int &core, const unsigned int &stage){
	if (allCoresFreeTimes[stage][core]==0) return 0;
	int currentStage = stage;
	while (currentStage < stages-1 && allCoresFreeTimes[currentStage][core] == delta) currentStage++;
	return currentStage*delta + allCoresFreeTimes[currentStage][core];
}

int  ResourceType::GetLeftBorderForOneCore(const unsigned int &core, const unsigned int &stage){
	if (allCoresFreeTimes[stage][core]==0) return 0;
	int currentStage = stage;
	while (currentStage-1 >= 0 && allCoresFreeTimes[currentStage-1][core] == delta) 
		currentStage--;
	return currentStage*delta;
}

bool ResourceType::IsDifferentResources(int core1, int core2){
	return (core1/numCoresPerOneRes == core2/numCoresPerOneRes);
}

/*
parameters:
timeCores - pairs of (execTime, coreCount)
stage - stage number
oneTypeCoreNums - vector of timeCores.size() size which contains LOCAL core numbers for resource type after concretizing
isCheckedForState = true if ResourceType::Check() is called in CheckState(). 
This means than we must find stages of beginning for all packages and fullfill oneTypeCoreNums with state concretizing.
Otherwise, oneTypeCoreNums contains cores for controls
addForbiddenCoreNums - forbidden cores from state or previous desicions in DirectBellman()
*/
bool ResourceType::Check(const vector<pair<double,unsigned int>>& timeCores, const int &stage,  
	vector<vector<int>>& oneTypeCoreNums, bool isCheckedForState){
	// all used core numbers on this stage
	vector <int> usedNums;
	vector <int>& coreTimes = allCoresFreeTimes[stage];
	vector <vector<int>> possiblePackagesCores;
	oneTypeCoreNums.resize(timeCores.size());
	possiblePackagesCores.resize(timeCores.size());
	
	// indexes of packages in timeCores sorted in descending order by time
	vector <int> packageIndexesTimeDesc;
	
	vector <int> rightBorders, leftBorders;
	for (int i = 0; i < GetCoresCount(); i++) 
		rightBorders.push_back(GetRightBorderForOneCore(i,stage));

	
	// forbidden cores
	for (vector<int>::iterator it = begin(addForbiddenCoreNums); it != end(addForbiddenCoreNums); it++){
			usedNums.push_back(*it);
	}
		
	if (isCheckedForState){
		for (int i = 0; i < GetCoresCount(); i++) 
			leftBorders.push_back(GetLeftBorderForOneCore(i,stage));
	}
	
	// find the pool of possible core numbers for each package
	for (vector<pair<double,unsigned int>>::size_type i = 0; i < timeCores.size(); i++){
		int minTime = T, minIndex = -1;
		for (vector<pair<double,unsigned int>>::size_type sort = i; sort < timeCores.size(); sort++){
			if (timeCores[sort].first < minTime && find(packageIndexesTimeDesc.begin(), packageIndexesTimeDesc.end(), sort) == packageIndexesTimeDesc.end()){
				minTime = timeCores[sort].first;
				minIndex = sort;
			}
		}
		packageIndexesTimeDesc.push_back(minIndex);
		// j - number of core
		for (int j = 0; j < GetCoresCount(); j++) {
			// if core isn't busy on this stage
			if (rightBorders[j] != 0 && find (usedNums.begin(), usedNums.end(), j) == usedNums.end()){
				// if package can be placed on this core, add core number to possiblePackagesCores
				if (isCheckedForState){
					if (leftBorders[j] + timeCores[i].first <= rightBorders[j] || rightBorders[j]==T)
						possiblePackagesCores[i].push_back(j);
				}
				else {
					if (stage * delta + timeCores[i].first <= rightBorders[j] || rightBorders[j]==T)
						possiblePackagesCores[i].push_back(j);
				}
			}
		}
		// ordering in rightBorders[j] asc
		for (int i1 = 0; i1 < possiblePackagesCores[i].size()-1; i1++){
			for (int i2 = i1+1; i2 < possiblePackagesCores[i].size(); i2++){
				if (rightBorders[possiblePackagesCores[i][i1]] > rightBorders[possiblePackagesCores[i][i2]]){
					int swap = possiblePackagesCores[i][i1];
					possiblePackagesCores[i][i1] = possiblePackagesCores[i][i2];
					possiblePackagesCores[i][i2] = swap;
				}
			}
		}
	}
	


	// for each packages
	for (vector <vector<int>>::size_type i = 0; i < packageIndexesTimeDesc.size(); i++){
		int index = packageIndexesTimeDesc[i];
		double time = timeCores[index].first;
		int coreNum = timeCores[index].second;
		int numStages = time/delta;
		if (time < delta) numStages++;
		else if ((int)time % delta!=0) numStages++;
		int coresViewed = 0, numberAdopted = 0;
		vector<int> onePackageUsedNums;
		for (int k = 0; k < coreNum; k++){
			for (unsigned int j = 0; j < possiblePackagesCores[index].size(); j++){
				int currentCore = possiblePackagesCores[index][j];
				if (find (usedNums.begin(), usedNums.end(), currentCore) == usedNums.end()){
						numberAdopted++;
						coresViewed++;
						onePackageUsedNums.push_back(j);
							
						if (canExecuteOnDiffResources==false){
						// if currentCore is not last in possiblePackageCores
							if (j!=possiblePackagesCores[index].size()-1
								// and next core is on different resource
								&& IsDifferentResources(possiblePackagesCores[index][j+1],currentCore)
								// and concretization was not found
								&& numberAdopted != coreNum) {
								coresViewed = 0;
								numberAdopted = 0;
								onePackageUsedNums.clear();
							}
					}
				}
				if (numberAdopted==coreNum) {
					oneTypeCoreNums[index] = onePackageUsedNums;
					break;
				}
			}
			
		}
		if (numberAdopted!=coreNum) return false;
	}
			
	return true;
}

void ResourceType::AddDiaps(int stageBegin, int stageCount, vector<int>& cores){
	for (vector<int>::iterator it = cores.begin(); it!= cores.end(); it++){
		int resourceIndex = GetResourceIndex(*it);
		int coreNum = *it - resourceIndex * numCoresPerOneRes;
		resources[resourceIndex].AddDiap(stageBegin, stageCount, coreNum);
	}
	SetFreeTimeEnds();
}

int ResourceType::GetResourceIndex(int core){
	int current = 0, resIndex = 0;
	while (core >= resIndex * numCoresPerOneRes)
		resIndex++;
	return resIndex-1;
}

void ResourceType::SetInitBusyIntervals(){
	for (vector<Resource>::iterator it = resources.begin(); it!= resources.end(); it++)
		it->SetInitBusyIntervals();
}

