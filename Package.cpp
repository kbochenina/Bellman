#include "StdAfx.h"
#include "Package.h"


Package::Package(int u, vector <int> &r, vector <int> &c, map <pair <int,int>, float> &e)
{
	uid = u;
	resourceTypes = r;
	coreCounts = c;
	execTime = e;
}

float Package::GetLevel(int stateNum) {
	try {
		string errMsg = "Package::GetLevel() error: package " + to_string((long long)uid) + " - unable to get stateNum " +
			to_string((long long)stateNum);
		if (stateNum < 0 || stateNum > packageStates.size() - 1) throw errMsg;
		return packageStates[stateNum].get<2>();
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

int Package::GetCore(int stateNum) {
	try {
		string errMsg = "Package::GetCore() error: package " + to_string((long long)uid) + " - unable to get stateNum " +
			to_string((long long)stateNum);
		if (stateNum < 0 || stateNum > packageStates.size() - 1) throw errMsg;
		return packageStates[stateNum].get<1>();
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

int Package::GetType(int stateNum) {
	try {
		string errMsg = "Package::GetType() error: package " + to_string((long long)uid) + " - unable to get stateNum " +
			to_string((long long)stateNum);
		if (stateNum < 0 || stateNum > packageStates.size() - 1) throw errMsg;
		return packageStates[stateNum].get<0>();
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

void Package::SetPackageStates(){
	try {
		string errTimeNotFound = "No execution time for package " + uid;
		if (!isInit) packageStates.push_back(make_tuple(0, 0, -1));
		packageStates.push_back(make_tuple(0, 0, 0));
		
		for (int i = 0; i < resourceTypes.size(); i++){
			for (int j = 0; j < coreCounts.size(); j++){
				int &resType = resourceTypes[i], &coreCount = coreCounts[j];
				map<pair<int,int>,float>::iterator exIt = execTime.find(make_pair(resType, coreCount));
				if (exIt == execTime.end()){
					errTimeNotFound += "(type " + to_string(long long (resType)) + ", cores " +
						to_string(long long (coreCount)) + ")";
					throw errTimeNotFound;
				}
				
				float &execTime = exIt->second;
				float f = 1.00/((int)execTime/delta + 1);
				for (float k = f; k < 1; k+= f)
					packageStates.push_back(make_tuple(resType, coreCount, k));
			}
		}
		packageStates.push_back(make_tuple(0, 0, 1));
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

void Package::PrintState(ofstream & f, int &state){
	f << "(" << packageStates[state].get<0>() << " " << packageStates[state].get<1>() << " " <<
		packageStates[state].get<2>() << ")";
}

int Package::GetNextStateNum(int currentStateNum, int controlType, int controlCore){
	try{
		string errCurrentStateNum = "Package::GetNextStateNum() error, wrong current state num";
		if (currentStateNum < 0 || currentStateNum > packageStates.size()-1) throw errCurrentStateNum;
		tuple <int,int, float> currentState = packageStates[currentStateNum];
		int currentType = currentState.get<0>();
		if (currentType==-1) return currentStateNum;
		if (currentType!=0) {
			// if we have next non-1 level for this (type, core)
			if (packageStates[currentStateNum+1].get<0>() == currentType && 
				packageStates[currentStateNum+1].get<1>() == packageStates[currentStateNum].get<1>())
			return currentStateNum + 1;
			else return packageStates.size()-1;
		}
		if (currentType==0){
			for (int i = currentStateNum + 1; i < packageStates.size(); i++){
				if (controlType == packageStates[i].get<0>() && controlCore == packageStates[i].get<1>())
					return i;
			}
			return packageStates.size()-1;
		}
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

Package::~Package(void)
{
}
