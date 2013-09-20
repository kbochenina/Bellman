#include "stdafx.h"
#include <map>
#include <vector>

extern int T, delta, stages;
extern vector<int> stageBorders;

class Resource{
	int number;
	int coresCount;
	std::vector<unsigned int> forcedBricks;
	std::map <int,std::vector<std::pair<int,int>>> busyIntervals; // (coreNum), (tBegin, tEnd), (tbegin, tend) for busy intervals
	std::map <int,std::vector<std::pair<int,int>>> currentBusyIntervals;
public:
	Resource(int,int,std::map <int,std::vector<std::pair<int,int>>>);
	std::map <int,std::vector<std::pair<int,int>>>* GetBusyIntervals(){return &busyIntervals;}
	int GetCoresCount(){return coresCount;}
	void CorrectBusyIntervals(std::vector<int>& const);
	// WHY / WHAA
	std::vector <int>* GetForcedNumbers();
	int GetNearestBorder(int coreNum, int stageBegin);
	void GetFreeTime(std::vector <std::vector<int>> & vec);
	int GetPlacement(const int&, const int&, const int&, std::vector<int>&);
	int GetCoreNearestBorder(const int&, const int&);
	void ResetBusyIntervals(){currentBusyIntervals = busyIntervals;}
	void SetForcedBricks();
	void AddForcedBricks(const std::vector<unsigned int>&);
	std::vector <unsigned int> GetNextStage(const std::vector<unsigned int> &);
	std::vector <unsigned int> GetForcedBricks(){return forcedBricks;}
};