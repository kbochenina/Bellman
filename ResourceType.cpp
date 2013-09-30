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

int ResourceType::GetNearestBorderForOneCore(const unsigned int &core, const unsigned int &stage){
	if (allCoresFreeTimes[stage][core]==0) return 0;
	int currentStage = stage;
	while (currentStage < stages-1 && allCoresFreeTimes[currentStage][core] == delta) currentStage++;
	return currentStage*delta + allCoresFreeTimes[currentStage][core];
}

bool ResourceType::Check(const vector<pair<double,unsigned int>>& timeCores, const int &stage, bool canExecuteOnDiffResources, 
	vector <pair<vector<int>,vector<int>>>&fullUsedNums, bool isUsedNumsNeeded, vector<vector<int>>& stageUsedNums){
	vector <int> usedNums;
	vector <int>& coreTimes = allCoresFreeTimes[stage];
	vector <int> nearestBorders;
	for (int i = 0; i < GetCoresCount(); i++) nearestBorders.push_back(GetNearestBorderForOneCore(i,stage));
	//if (isUsedNumsNeeded){
		// copy to usedNums busy cores received on previous stages
	vector <pair<vector<int>,vector<int>>>::iterator fIt = fullUsedNums.begin();
	for (;fIt!=fullUsedNums.end(); fIt++){
		if (find(fIt->first.begin(),fIt->first.end(),stage)!=fIt->first.end())
			copy(fIt->second.begin(), fIt->second.end(),back_inserter(usedNums));
	}
	//}
	// for each packages
	for (unsigned int i = 0; i < timeCores.size(); i++){
		vector <int> usedNumsForOnePackage;
		double time = timeCores[i].first;
		int coreNum = timeCores[i].second;
		int numStages = time/delta;
		if (time < delta) numStages++;
		else if ((int)time % delta!=0) numStages++;
		int coresViewed = 0, numberAdopted = 0;
		for (int k = 0; k < coreNum; k++){
			for (unsigned int j = 0; j < coreTimes.size(); j++){
				// if this core is avaliable
				if (nearestBorders[j]!=0){
					// if the core has not been used previously
					if (find (usedNums.begin(), usedNums.end(), j) == usedNums.end()){
						// if we have enough time on this core or nearestBorder is equal to T
						if (nearestBorders[j]-stage*delta >= time || nearestBorders[j] == T){
							numberAdopted++;
							coresViewed++;
							usedNums.push_back(j);
							usedNumsForOnePackage.push_back(j);
						}
						if (canExecuteOnDiffResources==false){
							if (coresViewed==numCoresPerOneRes-1) {
								coresViewed = 0;
								if (numberAdopted!=coreNum) numberAdopted = 0;
							}
						}
					}
				}
				if (numberAdopted==coreNum) break;
			}
		}
		if (numberAdopted!=coreNum) return false;
		
		vector <int> numStagesVec;
		for (int j = 0; j < numStages; j++) numStagesVec.push_back(stage+j);
		fullUsedNums.push_back(make_pair(numStagesVec,usedNumsForOnePackage));

		if (isUsedNumsNeeded) {
			stageUsedNums.push_back(usedNumsForOnePackage);
		}
		
		// for (unsigned int j = 0; j < usedNums.size(); j++) coreTimes[usedNums[j]]=0;
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

