#include "stdafx.h"
#include "Resource.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include "UserException.h"
using namespace std;



typedef map <int,vector<pair<int,int>>>::iterator iInterval;

bool avaliableTimesAsc(const std::pair <int, int> &p1, const std::pair <int, int> &p2)
{
	return p1.second < p2.second;
}

bool busyIntervalsAsc(const std::pair <int, int> &p1, const std::pair <int, int> &p2)
{
	return p1.first < p2.first;
}

struct nearest{
	explicit nearest(int n): number(n){}
	inline bool operator () (pair<int,int> obj){
		return (obj.first >= number);
	}
private:
	int number;
};

struct findBegin{
	explicit findBegin(int n): number(n){}
	inline bool operator () (pair<int,int> obj){
		return (obj.first == number);
	}
private:
	int number;
};

struct findEnd{
	explicit findEnd(int n): number(n){}
	inline bool operator () (pair<int,int> obj){
		return (obj.second == number);
	}
private:
	int number;
};

struct findIntersectsDiap{
	explicit findIntersectsDiap(int tb, int te): tbegin(tb), tend(te){}
	inline bool operator () (pair<int,int> obj){
		bool res = false;
		if (obj.first <= tbegin && obj.second > tbegin) res = true;
		if (obj.first < tend && obj.second >=tend) res = true;
		if (obj.first==tbegin && obj.second==tend) res = false;
		return (res);
	}
private:
	int tbegin, tend;
};

struct findIntersects{
	explicit findIntersects(int tb): tbegin(tb){}
	inline bool operator () (pair<int,int> obj){
		bool res = false;
		if (obj.first <= tbegin && obj.second > tbegin) res = true;
		return (res);
	}
private:
	int tbegin;
};


struct isIn{
	explicit isIn(int n): number(n){}
	inline bool operator () (pair<int,int> obj){
		return (obj.first <= number && obj.second >number);
	}
private:
	int number;
};


void Resource::SetForcedBricks(){
	int shift = T/delta - 1;
	for (unsigned int i = 0; i < forcedBricks.size(); i++) 
		forcedBricks[i]=0;
	for (int i = 0; i < coresCount; i++){
		vector <pair<int,int>>::iterator diap = currentBusyIntervals[i+1].begin();
		for (; diap!= currentBusyIntervals[i+1].end(); diap++){
			int begin = diap->first;
			int end = diap->second;
			int beginBitNum = 0;
			if (begin % delta == 0) 
				beginBitNum = begin / delta;
			else beginBitNum = (begin/delta)+1;
			int endBitNum = end / delta;
			for (int j = beginBitNum; j < endBitNum; j++)
				forcedBricks[i] |= 1 << (shift-j);
			
		}
	}
	ResetBusyIntervals();
}

void Resource::AddForcedBricks(const vector<unsigned int>& add){
	try{
		int shift = T/delta - 1;
		if (add.size()!= forcedBricks.size())
			throw "AddForcedBricks error";
		vector <unsigned int> victim; 
		for (unsigned int i = 0; i < coresCount; i++){
			victim.push_back(add[i]);
			forcedBricks[i]|=add[i];
			int currentBit = 0;
			vector <int> diapNumbers;
			int prevVictim = add[i];
			for (int j = 0; j < shift + 1; j++){
				currentBit = victim[i] >> (shift-j);
				if (currentBit==1) diapNumbers.push_back(j);
				victim[i]=prevVictim;
				int mask = (int)pow((double)2,shift-j) ^ 0xFF;
				victim[i] &= mask;
				prevVictim = victim[i];
			}
			if (diapNumbers.size()!=0){
				int currentDiapBegin = diapNumbers[0], currentDiapEnd = currentDiapBegin;
				for (unsigned int j = 1; j < diapNumbers.size(); j++){
					if (diapNumbers[j] == currentDiapEnd + 1){
						currentDiapEnd = diapNumbers[j];
					}
					else {
						vector<pair<int,int>>::iterator it = find_if(currentBusyIntervals[i+1].begin(), 
															currentBusyIntervals[i+1].end(), findIntersects(currentDiapEnd*delta+delta));
						if (it!=currentBusyIntervals[i+1].end()) it->first = currentDiapBegin*delta;
						else{
							currentBusyIntervals[i+1].push_back(make_pair(currentDiapBegin*delta, currentDiapEnd*delta + delta));
							currentDiapBegin = diapNumbers[j];
							currentDiapEnd = currentDiapBegin;
						}
					}
				}
				vector<pair<int,int>>::iterator it = find_if(currentBusyIntervals[i+1].begin(), 
					currentBusyIntervals[i+1].end(), findIntersects(currentDiapEnd*delta+delta));
				if (it!=currentBusyIntervals[i+1].end()) it->first = currentDiapBegin*delta;
				else
					currentBusyIntervals[i+1].push_back(make_pair(currentDiapBegin*delta, currentDiapEnd*delta + delta));
			}
			sort(currentBusyIntervals[i+1].begin(), currentBusyIntervals[i+1].end(), busyIntervalsAsc);
			
		}
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

Resource::Resource(int n,int c, map <int,vector<pair<int,int>>> b)
{
	number = n;
	coresCount = c;
	initBusyIntervals = b;
	currentBusyIntervals = initBusyIntervals;
	onlyFirstBusyIntervals = b;
	for (unsigned int i = 0; i < coresCount; i++) forcedBricks.push_back(0);
}



void Resource::CorrectBusyIntervals(const std::vector<int>&  vec){
	
	for (iInterval it = initBusyIntervals.begin(); it!= initBusyIntervals.end(); it++){
		vector <int> toEraseNums;
		int newVal = 0; 
		for (vector<pair<int,int>>::iterator it2 = it->second.begin(); it2!=it->second.end(); it2++){
			if (newVal > it2->first){
				it2->first = (it2-1)->first;
				toEraseNums.push_back(distance(it->second.begin(), it2-1));
			}
			int bEnd = it2->second;
			for (unsigned int i = 0; i < vec.size()-1; i++) {
				if (vec[i] < bEnd && vec[i+1] > bEnd) {
					newVal = it2->second = vec[i+1];
				}
			}
		}
		for (unsigned int i = 0; i < toEraseNums.size(); i++){
			it->second.erase(it->second.begin()+toEraseNums[i]);
		}
	}
	currentBusyIntervals = initBusyIntervals;
}

std::vector <int>* Resource::GetForcedNumbers(){
	vector <int> *forcedNumbers = new vector <int>;
	try{
		if (stageBorders.size()<1) throw "GetForcedNumbers error";
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	
	for (iInterval it = currentBusyIntervals.begin(); it!= currentBusyIntervals.end(); it++){
		int coreNum = it->first;
		for (vector<pair<int,int>>::iterator it2 = it->second.begin(); it2!=it->second.end(); it2++){
		int bBegin = it2->first;
		int bEnd = it2->second;
		for (unsigned int i = 0; i < stageBorders.size(); i++) {
			if (stageBorders[i] <= bBegin && bBegin - stageBorders[i]<= delta) {
				int index = 0, length = 0;
				if (stageBorders[i]== bBegin) {
					index = i; length = (bEnd-stageBorders[i])/delta;
				}
				else {
					index = i+1;
					length = (bEnd-stageBorders[i+1])/delta;
				}
				int coreBegin = (coreNum-1)* (stageBorders.size()-1);
				for (int j = 0; j < length; j++)
					forcedNumbers->push_back(coreBegin + 1 + index++);
			}
			
			}
		}
	}
	return forcedNumbers;
}

int Resource::GetNearestBorder(unsigned int coreNum, int stageBegin){
	vector<int> nearestBorders;
	for (iInterval it = currentBusyIntervals.begin(); it!=currentBusyIntervals.end(); it++){
		if (it->second.size()==0) nearestBorders.push_back(T);
		vector<pair<int,int>>::iterator it2 = std::find_if(it->second.begin(),it->second.end(),isIn(stageBegin));
		if (it2==it->second.end()){
		it2 = std::find_if(it->second.begin(),it->second.end(),nearest(stageBegin));
		if (it2!=it->second.end()) nearestBorders.push_back(it2->first);
		else nearestBorders.push_back(T);
		}
	}
	if (nearestBorders.size()<coreNum) return 0;
	sort(nearestBorders.begin(), nearestBorders.end());
	reverse(nearestBorders.begin(), nearestBorders.end());
	int min = nearestBorders[0];
	return min;
}

// ((core1 freeTime, core2 FreeTime), (core 1 freeTime, core2 FreeTime)) - stage 
void Resource::GetFreeTime(std::vector <std::vector<int>> & vec){
	try{
		if (vec.size() < (unsigned int)T/delta) throw "Resource::GetFreeTime() - vector has not enough stages";
		for (unsigned int i = 0; i < vec.size(); i++)
			if (vec[i].size() < coresCount) throw "Resource::GetFreeTime() - vector has not enough core counts";
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
		
	}
	int resourceIndex = 0;
	for (iInterval it = currentBusyIntervals.begin(); it!= currentBusyIntervals.end(); it++){
		if (it->second.size()==0) {
			for (int i = 0; i < T; i+=delta)
				vec[i/delta][resourceIndex] = delta;
			resourceIndex++;
			continue;
		}
		vector <pair<int, int>> intervals = it->second;
		int indexBusyInterval = 0;
		for (int i = 0; i < T; i+=delta){
			int bBegin = intervals[indexBusyInterval].first;
			int bEnd = intervals[indexBusyInterval].second;
			if (bEnd % delta!=0) {
				bEnd = (bEnd/delta + 1) * delta;
			}
			if (resourceIndex > (int)vec[i/delta].size()-1) 
				throw UserException("Resource::GetFreeTime(): wrong vec[i/delta] size");
			if (i < bBegin && i+delta <= bBegin) vec[i/delta][resourceIndex] = delta;
			else {
				vec[i/delta][resourceIndex] = bBegin - i;
				i+=delta;
				while(i!=bEnd) {
					if (i/delta > vec.size()-1) {
						vector <pair<int, int>>::iterator it = intervals.begin();
						for (; it!= intervals.end(); it++){
							cout << it->first << " " << it->second << endl;
						}
						cout << "bBegin = " << bBegin << endl;
						cout << "bEnd = " << bEnd << endl;
						throw UserException("Resource::GetFreeTime(): wrong i/delta");
					}
					vec[i/delta][resourceIndex] = 0;
					i+=delta;
				}
				
				indexBusyInterval++;
				if (indexBusyInterval == intervals.size())
					while (i<T) {
						vec[i/delta][resourceIndex] = delta;
						i+=delta;
					}
				i-=delta;
			}
		}
		resourceIndex++;
	}
}

int Resource::GetCoreNearestBorder(const int& coreNum, const int&tBegin){
	int diapCount = currentBusyIntervals[coreNum+1].size();
	if (diapCount == 0) return T;
	for (int i = 0; i < diapCount; i++){
		int intervalBegin = currentBusyIntervals[coreNum+1][i].first;
		int intervalEnd = currentBusyIntervals[coreNum+1][i].second;
		if (tBegin >= intervalBegin && tBegin < intervalEnd) return 0;
		if (tBegin < intervalBegin) return intervalBegin;
	}
	return T;
}

// function don't change resource forcedBricks
std::vector <unsigned int> Resource::GetNextStage(const std::vector<unsigned int> & control){
	try{
		if (control.size()!= forcedBricks.size())
			throw "Resource::GetNextStage() error";
		vector <unsigned int> victim = forcedBricks;
		for (unsigned int i = 0; i < victim.size(); i++){
			victim[i] |= control[i];
		}
		return victim;
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

int Resource::GetPlacement(const int& tBegin, const int& execTime, const unsigned int& numCores, vector<int>&realCoreNums){
	if (numCores > coresCount) return -1;
	vector <pair<int,int>> coresAvaliableTimes, uncompletedTimes;
	for (unsigned int i = 0; i < coresCount; i++) {
		vector<pair<int,int>>::iterator it = find_if(currentBusyIntervals[i+1].begin(), currentBusyIntervals[i+1].end(), isIn(tBegin));
		if (it==currentBusyIntervals[i+1].end()){
			int nearestBorder = GetCoreNearestBorder(i,tBegin);
			int avaliableTime = nearestBorder-tBegin;
			if (avaliableTime >= execTime) {
				coresAvaliableTimes.push_back(make_pair(i,avaliableTime));
			}
			if (nearestBorder==T && avaliableTime < execTime){
				uncompletedTimes.push_back(make_pair(i,avaliableTime));
			}
		}
	}
	if (coresAvaliableTimes.size() + uncompletedTimes.size() < numCores) return -1;
	sort(coresAvaliableTimes.begin(), coresAvaliableTimes.end(), avaliableTimesAsc);
	sort(uncompletedTimes.begin(), uncompletedTimes.end(), avaliableTimesAsc);
	
	int distance = 0;
	
	for (unsigned int i = 0; i < numCores;i++){
		if (i < coresAvaliableTimes.size()){
			realCoreNums[i] = coresAvaliableTimes[i].first;
			distance += coresAvaliableTimes[i].second - execTime;
		}
		else{
			realCoreNums[i] = uncompletedTimes[i + coresAvaliableTimes.size()].first;
			distance += 0;
		}
	}
	return distance;
}

void Resource::AddDiap(int stageBegin, int execTime, int coreNum){
	try{
		if (coreNum < 0 || coreNum > coresCount - 1) throw UserException("Resource::AddDiap() : wrong core number" + to_string((long long)coreNum));
		int tbegin = stageBegin*delta;
		if (tbegin%delta!=0) throw UserException("Resource::AddDiap() : fraction tbegin");
		int tend = tbegin+execTime;
		if (tend > T) tend = T;

		coreNum++;

		if (find_if(currentBusyIntervals[coreNum].begin(), currentBusyIntervals[coreNum].end(), findIntersectsDiap(tbegin, tend))
			!=currentBusyIntervals[coreNum].end())
			throw UserException("Resource::AddDiap() : attempt to add intersects diaps");

		// expand tend to stage border
		if (tend%delta!=0){
			int stageNum = tend/delta;
			tend = (stageNum + 1)*delta;
		}

		bool changedExistedDiap = false;
		vector<pair<int,int>>::iterator currentDiap = currentBusyIntervals[coreNum].begin();
		// if tbegin is equal to end of some diapason, that diapason will be extended
		auto it1 = find_if(currentBusyIntervals[coreNum].begin(), currentBusyIntervals[coreNum].end(),findEnd(tbegin));
		if (it1!=currentBusyIntervals[coreNum].end()){
			it1->second = tend;
			changedExistedDiap = true;
			currentDiap = it1;
		}

		auto it2 = find_if(currentBusyIntervals[coreNum].begin(), currentBusyIntervals[coreNum].end(),findBegin(tend));
		if (it2!=currentBusyIntervals[coreNum].end()){
			if (changedExistedDiap){
				it1->second = it2->second;
				currentBusyIntervals[coreNum].erase(it2);
			}
			else {
				it2->first = tbegin;
				changedExistedDiap = true;
				currentDiap = it2;
			}
		}

		if (changedExistedDiap)
		{
			tbegin = currentDiap->first;
			tend = currentDiap->second;
		}

		auto intersect = 
			find_if(currentBusyIntervals[coreNum].begin(), currentBusyIntervals[coreNum].end(), findIntersectsDiap(tbegin, tend));
		if (intersect!=currentBusyIntervals[coreNum].end()){
			if (changedExistedDiap){
				currentDiap->second = intersect->second;
				currentBusyIntervals[coreNum].erase(intersect);
			}
			else {
				intersect->first = tbegin;
				changedExistedDiap = true;
			}
		}
		
		
		pair <int,int> newDiap;
		newDiap.first = tbegin; newDiap.second = tend;
				
		if (tend%delta!=0) 
			throw UserException("Resource::AddDiap() : fraction tend");
		if (coreNum == 0)
			throw UserException("Resource::AddDiap() : zero value of coreNum");
		if (!changedExistedDiap) 
			currentBusyIntervals[coreNum].push_back(newDiap);
		sort(currentBusyIntervals[coreNum].begin(), currentBusyIntervals[coreNum].end(), busyIntervalsAsc);
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}