#include "StdAfx.h"
#include "Package.h"
#include "UserException.h"

Package::Package(int u, vector <int> &r, vector <int> &c, map <pair <int,int>, double> &e)
{
	uid = u;
	resourceTypes = r;
	coreCounts = c;
	execTime = e;
}

double Package::GetLevel(unsigned int stateNum) {
	try {
		string errMsg = "Package::GetLevel() error: package " + to_string((long long)uid) + " - unable to get stateNum " +
			to_string((long long)stateNum);
		if (stateNum < 0 || stateNum > packageStates.size() - 1) throw UserException(errMsg);
		return packageStates[stateNum].get<2>();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

int Package::GetCore(unsigned int stateNum) {
	try {
		string errMsg = "Package::GetCore() error: package " + to_string((long long)uid) + " - unable to get stateNum " +
			to_string((long long)stateNum);
		if (stateNum < 0 || stateNum > packageStates.size() - 1) throw UserException(errMsg);
		return packageStates[stateNum].get<1>();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

int Package::GetType(unsigned int stateNum) {
	try {
		string errMsg = "Package::GetType() error: package " + to_string((long long)uid) + " - unable to get stateNum " +
			to_string((long long)stateNum);
		if (stateNum < 0 || stateNum > packageStates.size() - 1) throw UserException(errMsg);
		return packageStates[stateNum].get<0>();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Package::SetPackageStates(){
	try {
		string errTimeNotFound = "No execution time for package " + uid;
		if (!isInit) packageStates.push_back(make_tuple(0, 0, -1));
		packageStates.push_back(make_tuple(0, 0, 0));
		
		for (unsigned int i = 0; i < resourceTypes.size(); i++){
			for (unsigned int j = 0; j < coreCounts.size(); j++){
				int &resType = resourceTypes[i], &coreCount = coreCounts[j];
				map<pair<int,int>,double>::iterator exIt = execTime.find(make_pair(resType, coreCount));
				if (exIt == execTime.end()){
					errTimeNotFound += "(type " + to_string(long long (resType)) + ", cores " +
						to_string(long long (coreCount)) + ")";
					throw UserException(errTimeNotFound);
				}
				
				double &execTime = exIt->second;
				double f = 1.00/((int)execTime/delta + 1);
				for (double k = f; k < 1; k+= f)
					packageStates.push_back(make_tuple(resType, coreCount, k));
			}
		}
		packageStates.push_back(make_tuple(0, 0, 1));
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Package::PrintState(ofstream & f, const int &state){
	f << "(" << packageStates[state].get<0>() << " " << packageStates[state].get<1>() << " " <<
		packageStates[state].get<2>() << ")";
}

int Package::GetNextStateNum(unsigned int currentStateNum, int controlType, int controlCore){
	try{
		string errCurrentStateNum = "Package::GetNextStateNum() error, wrong current state num";
		if (currentStateNum < 0 || currentStateNum > packageStates.size()-1) throw UserException(errCurrentStateNum);
		tuples::tuple <int,int, double> currentState = packageStates[currentStateNum];
		int currentType = currentState.get<0>();
		if (currentType==-1) return currentStateNum;
		if (currentType!=0) {
			// if we have next non-1 level for this (type, core)
			if (packageStates[currentStateNum+1].get<0>() == currentType && 
				packageStates[currentStateNum+1].get<1>() == packageStates[currentStateNum].get<1>())
			return currentStateNum + 1;
			else return packageStates.size()-1;
		}
		else {
			for (unsigned int i = currentStateNum + 1; i < packageStates.size(); i++){
				if (controlType == packageStates[i].get<0>() && controlCore == packageStates[i].get<1>())
					return i;
			}
			return packageStates.size()-1;
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}


double Package::GetExecTime(int type, int cores){
	try{
		std::pair<int,int> typeCore = make_pair(type,cores);
		auto it = execTime.find(typeCore);
		if (it==execTime.end()) 
			throw UserException("Package::GetExecTime() : combination of type " + to_string((long long)type) + 
			"  and cores " + to_string((long long)cores) + " not found");
		return execTime[std::make_pair(type,cores)];
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

double Package::GetPartialExecTime(const unsigned int & stateNum){
	try{
		string errMsg = "GetPartialExecTime() error, diff can`t be less than zero";

		unsigned int type = GetType(stateNum);
		unsigned int cores = GetCore(stateNum);
		double readyLevel = GetLevel(stateNum);
		double execTime =  GetExecTime(type,cores);
		double approxReadyTime = execTime * readyLevel;
		int readyStages = (int)approxReadyTime/delta;
		if ((int)approxReadyTime % delta !=0) readyStages++;
		double realTime = readyStages * delta;
		double diff = approxReadyTime-realTime;
		if (diff < 0) return (execTime - delta * readyStages);
		return (diff + (1-readyLevel)*execTime);
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

double Package::GetExecTime(const unsigned int & stateNum){
	return GetExecTime(GetType(stateNum),GetCore(stateNum));
}

void Package::PrintExecTime(ofstream &f){
	f << "Package " << to_string((long long)uid) << endl;
	for (map <pair <int,int>, double>::iterator i = execTime.begin(); i != execTime.end(); i++ ){
		f << "(" << i->first.first << " " <<  i->first.second << " " << i->second << ")";
		f << endl;
	}
}

Package::~Package(void)
{
}
