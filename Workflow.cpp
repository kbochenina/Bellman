#include "stdafx.h"
#include "Workflow.h"
#include "time.h"


Workflow::Workflow (std::vector <Package*> p, std::vector <std::vector <int>> c, int w, vector <ResourceType*>& r) : _refResources(r){
	packages = p;
	connectMatrix = c;
	wfNum = w;
	maxPossibleTime = 120.0 * 1000;
	elapsedTime = 0.0;
	
	// find count of zero rows
	int zeroCount = 0;
	for (int i = 0; i < packages.size(); i++){
		bool isZero = true;
		for (int j = 0; j < packages.size(); j++){
			if (connectMatrix[i][j]) isZero = false;
		}
		if (isZero) zeroCount++;
	}
	ex << "Zero count: " << zeroCount << endl;
}

void Workflow::SetPackagesStates(){
	for (int i = 0; i < packages.size(); i++)
		packages[i]->SetPackageStates();
}

void Workflow::SetIsPackageInit(){
	for (int i = 0; i < packages.size(); i++){
		bool isInit = true;
		for (int j = 0; j < packages.size(); j++)
			if (connectMatrix[j][i] == 1) {
				packages[i]->SetIsInit(false);
				isInit = false;
				break;
			}
		if (isInit) packages[i]->SetIsInit(true);
	}
}

void Workflow::SetFullPackagesStates(int currentPackage){
	if (currentPackage == 0) beginTime = clock();
	// if it is the last package
	try{
		string errWrongPackageNumber = "SetFullPackagesStates(): workflow " + to_string((long long)wfNum) 
			+ " has no package " + to_string((long long)currentPackage); 
		string notEnoughTime = "SetFullPackageStates() was terminated, time was more than " + to_string((long double)maxPossibleTime/1000.0) + " s";
		if (currentPackage == packages.size()-1) {
			vector <int> lastPackageStates;
			for (int i = 0; i < packages[currentPackage]->GetStatesCount(); i++) {
				lastPackageStates.push_back(i);
				packagesStates.push_back(lastPackageStates);
				lastPackageStates.clear();
			}
		}
		else {
			vector <int> currentPackageStates;
			vector <vector <int>> newStates;
			vector <int> newState;
			newState.resize(packages.size() - currentPackage);
			for (int i = 0; i < packages[currentPackage]->GetStatesCount(); i++) 
				currentPackageStates.push_back(i);
			SetFullPackagesStates(currentPackage+1);
			for (int i = 0; i < currentPackageStates.size(); i++){
				for (int j = 0; j < packagesStates.size(); j++){
					if (Check(currentPackageStates[i],packagesStates[j], currentPackage)) {
						newState[0] = currentPackageStates[i];
						for (int k = 1; k <= packagesStates[j].size(); k++)
							newState[k] = packagesStates[j][k-1];
						newStates.push_back(newState);
						if (currentPackage == 0) GetControls(newState, 0);
					}
					elapsedTime =(clock()-beginTime);
					if (elapsedTime > maxPossibleTime) throw notEnoughTime;
				}
			}
			packagesStates = newStates;
		}
	}
	catch(const string msg){
		cout << msg << endl;
		ex << msg << endl;
		//system("pause");
		exit(EXIT_FAILURE);
	}
}

void Workflow::GetControls(vector<int>& newState, int currentNum){
	if (currentNum == 0) oneStateControls.clear();
	if (currentNum == packages.size()-1){
		float level = packages[currentNum]->GetLevel(newState[currentNum]);
		if (level == -1 || level == 1) {
			vector <pair<int,int>> solution;
			solution.push_back(make_pair(0, 0));
			oneStateControls.push_back(solution);
		}
		else {
			vector <int> r, c;
			packages[currentNum]->GetResourceTypes(r);
			packages[currentNum]->GetCoreCounts(c);
		}

	}
}

bool Workflow::Check (const int& state, const vector <int> & otherStates, const int & currentPackage){
	int stateLevel = packages[currentPackage]->GetLevel(state);
	int core = packages[currentPackage]->GetCore(state);
	const int type = packages[currentPackage]->GetType(state);
	for (int i = 0; i < otherStates.size(); i++){
		const int &otherState = otherStates[i];
		const int & otherPackage = currentPackage + i + 1;
		int otherStateLevel = packages[otherPackage]->GetLevel(otherState);
		// if  otherPackage depends on currentPackage
		if (IsDepends(currentPackage, otherPackage)){
			if (stateLevel!=1 && otherStateLevel!=-1) return false;
			if (stateLevel==1 && otherStateLevel==-1){
				bool isOtherReady = true;
				for (int j = 0; j < otherStates.size(); j++){
					if ( i!=j && IsDepends(currentPackage + j + 1, otherPackage) && packages[currentPackage+j + 1]->GetLevel(otherStates[j])!=1)
						isOtherReady = false;
				}
				if (isOtherReady) return false;
			}
		}
		else {
			if (type){
				const int otherType = packages[otherPackage]->GetType(otherState);
				if (type == otherType){
					core += packages[otherPackage]->GetCore(otherState);
				}
			}
		}
		
	}
	if (type && core > _refResources[type-1]->GetCoresCount()) return false;

	return true;
}

void Workflow::PrintPackagesStates(){
	string f = "wf"+to_string((long long)wfNum) + ".txt";
	ofstream file (f);
	for (int i = 0; i < packagesStates.size(); i++){
		for (int j = 0; j < packagesStates[i].size(); j++){
			packages[j]->PrintState(file, packagesStates[i][j]);
			file << " ";
		}
		file << endl;
	}
	file.close();
}

bool Workflow::IsDepends(int one, int two){
	try {
		string errorMsg = "IsDepends() error (workflow " + to_string((long long) wfNum) + "): incorrect package num - ";
		if (one > packages.size()-1) throw errorMsg +  to_string((long long) one);
		if (two > packages.size()-1) throw errorMsg +  to_string((long long) two);
		if (connectMatrix[two][one]==1) return true;
		else {
			vector <int> notDirectDepends;
			for (int i = 0; i < connectMatrix[one].size(); i++){
				if (connectMatrix[one][i]!=0) 
					if (IsDepends(i,two)) return true;
			}
			return false;
		
		}
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

void Workflow::GetDependency(int pNum, vector<int>&res){
	for (int i = 0; i < connectMatrix[pNum].size(); i++){
		if (connectMatrix[i][pNum]==1) res.push_back(i);
	}
}

bool Workflow::IsPackageInit(int pNum){
	bool flag = true;
	for (int i = 0; i < connectMatrix.size(); i++){
		if (connectMatrix[i][pNum]!=0) return false;
	}
	return flag;
}


