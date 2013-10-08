#include "stdafx.h"
#include "Workflow.h"
#include "time.h"
#include <iostream>
#include "UserException.h"
using namespace std;


Workflow::Workflow (std::vector <Package*> p, std::vector <std::vector <int>> c, int w, vector <ResourceType*>& r, 
	vector <pair<unsigned short, unsigned short>> & tc) : _refResources(r), _reftypesCores(tc){
	packages = p;
	connectMatrix = c;
	wfNum = w;
	maxPossibleTime = 120.0 * 1000;
	elapsedTime = 0.0;
	// find count of zero rows
	int zeroCount = 0;
	for (unsigned int i = 0; i < packages.size(); i++){
		bool isZero = true;
		for (unsigned int j = 0; j < packages.size(); j++){
			if (connectMatrix[i][j]) isZero = false;
		}
		if (isZero) zeroCount++;
	}
	ex << "Zero count: " << zeroCount << endl;

}

void Workflow::SetPackagesStates(){
	for (unsigned int i = 0; i < packages.size(); i++)
		packages[i]->SetPackageStates();
}

void Workflow::SetIsPackageInit(){
	for (unsigned int i = 0; i < packages.size(); i++){
		bool isInit = true;
		for (unsigned int j = 0; j < packages.size(); j++)
			if (connectMatrix[j][i] == 1) {
				packages[i]->SetIsInit(false);
				isInit = false;
				break;
			}
		if (isInit) packages[i]->SetIsInit(true);
	}
}

void Workflow::SetFullPackagesStates(int currentPackage, vector <vector<int>>& packagesStates, vector <vector<vector<int>>>&controls, 
	vector <vector<int>> & nextStateNumbers){
	ofstream tf;
	double fctime = 0;
	if (currentPackage == 0) {
		beginTime = clock(), prevBeginTime = beginTime;
		string tfname = "wf" + to_string((long long)wfNum) + "_time.txt";
		tf.open(tfname);
	}
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
			SetFullPackagesStates(currentPackage+1, packagesStates, controls, nextStateNumbers);
		//	cout << "states for package " << currentPackage+1 << " finished (elapsed time - " << (clock()-prevBeginTime)/1000.0 << " sec)" << endl;
			prevBeginTime = clock();
			for (unsigned int i = 0; i < currentPackageStates.size(); i++){
				for (unsigned int j = 0; j < packagesStates.size(); j++){
					if (Check(currentPackageStates[i],packagesStates[j], currentPackage)) {
						newState[0] = currentPackageStates[i];
						for (unsigned int k = 1; k <= packagesStates[j].size(); k++)
							newState[k] = packagesStates[j][k-1];
						newStates.push_back(newState);
						if (currentPackage == 0){
							int ct = clock();
							GetControls(newState, 0); 
							tf << "Time of getting controls " << (clock()-ct)/1000.0 << endl;
							fctime+=(clock()-ct)/1000.0;
							tf << "oneStateControls.size() " << oneStateControls.size() << endl;
							controls.push_back(oneStateControls);
						}
					}
					elapsedTime =(clock()-beginTime);
					//if (elapsedTime > maxPossibleTime) throw notEnoughTime;
				}
			}
			
			packagesStates = newStates;
			if (currentPackage == 0) {
				tf << "Full time of getting controls " << fctime << endl;
				
				int ct = clock();
				SetNextStateNumbers(packagesStates, controls, nextStateNumbers);
				tf << "Time of setting next state numbers " << (clock()-ct)/1000.0 << endl;
				tf.close();
			}
		}
	}
	catch(const string msg){
		cout << msg << endl;
		ex << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

void Workflow::SetNextStateNumbers(vector <vector<int>>& packagesStates, vector <vector<vector<int>>>& controls, 
	vector <vector<int>>& nextStateNumbers){
	try{
		// i is also stateNumber
		for (unsigned int i = 0; i < nextPackageStateNumbers.size(); i++){
			vector <int> readyNumbers;
			for (unsigned int j = 0; j < nextPackageStateNumbers[i].size(); j++){
				vector<vector<int>>::iterator it = find(packagesStates.begin()+i, packagesStates.end(), nextPackageStateNumbers[i][j]);
				if (it==packagesStates.end()) throw UserException("SetNextStateNumbers() error: cannot find next state");
				// find next state number
				readyNumbers.push_back(distance(packagesStates.begin(),it));
				/*for (int stateIndex = i; stateIndex < packagesStates.size(); stateIndex++){
					bool flag = true;
					for (int k = 0; k < nextPackageStateNumbers[i][j].size(); k++){
						if (nextPackageStateNumbers[i][j][k] != packagesStates[stateIndex][k]) {
							flag = false; break;
						}
					}
					if (flag) { 
						readyNumbers.push_back(stateIndex); break;
					}
				}*/
			}
			nextStateNumbers.push_back(readyNumbers);
		}
		nextPackageStateNumbers.clear();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Workflow::GetControls(vector<int>& newState, unsigned int currentNum){
	try{
		string wrongIndexErr = "GetControls() error: wrong second argument (WF " + to_string((long long)wfNum) + "), packageNum = " +
				to_string((long long)currentNum);
		if (currentNum < 0 || currentNum > packages.size()-1 || currentNum > newState.size()-1) throw wrongIndexErr;
		if (currentNum == 0) oneStateControls.clear();
		vector <pair<int,int>> currentControl; 
		double level = packages[currentNum]->GetLevel(newState[currentNum]);
		if (level == -1 || level == 1) {
			currentControl.push_back(make_pair(-1,-1));
		}
		else if (level > 0){
			currentControl.push_back(make_pair(packages[currentNum]->GetType(newState[currentNum]), 
					packages[currentNum]->GetCore(newState[currentNum])));
		}
		else {
			vector <int> r, c;
			packages[currentNum]->GetResourceTypes(r);
			packages[currentNum]->GetCoreCounts(c);
			currentControl.push_back(make_pair(-1,-1));

			for (unsigned int i = 0; i < r.size(); i++){
				for (unsigned int j = 0; j < c.size(); j++){
					// if resourceType[r[i]-1] has enough cores
					if (c[j] <= _refResources[r[i]-1]->GetCoresCount()){
						currentControl.push_back(make_pair(r[i], c[j]));
					}
				}
			}
		}
		
		if (currentNum == packages.size()-1){
			for (unsigned int i = 0; i < currentControl.size(); i++) {
				vector <int> one;
				if (currentControl[i].first == -1) 
					one.push_back(-1);
				else one.push_back(_refResources[currentControl[i].first-1]->GetInitVal() + currentControl[i].second - 1);
				oneStateControls.push_back(one);
			}
			//if (currentControl.size() == 0) cout << "currentControl.size() == 0!" << endl;
		}
		else {
			GetControls(newState, currentNum+1);
			// check for compatibility
			vector <vector <int>> newControls;
			vector <vector <int>> nextPackagesStates;
			for (unsigned int i = 0; i < currentControl.size(); i++){
				for (unsigned int j = 0; j < oneStateControls.size(); j++){
					vector <int> newControl;
					vector <int> nextStates;
					if (currentControl[i].first == -1 || CheckCores(currentControl[i], oneStateControls[j])){
						if (currentControl[i].first == -1) {
							newControl.push_back(-1);
							if (currentNum == 0) 
								nextStates.push_back(newState[currentNum]);
						}
						else {
							newControl.push_back(_refResources[currentControl[i].first-1]->GetInitVal() + currentControl[i].second - 1);
							if (currentNum == 0) {
								int nextStateNum = packages[currentNum]->GetNextStateNum(newState[currentNum], currentControl[i].first,
									currentControl[i].second);
								nextStates.push_back(nextStateNum);
							}
						}
						for (unsigned int k = 0; k < oneStateControls[j].size(); k++) {
							int &indexVal = oneStateControls[j][k];
							newControl.push_back(indexVal);
							if (currentNum == 0){
								// if first package is ready, depend packages set to state (0,0) - readyness
								if (indexVal == -1) nextStates.push_back(newState[k+1]);
								else {
									int nextStateNum = packages[k+1]->GetNextStateNum(newState[k+1], _reftypesCores[indexVal].first,
									_reftypesCores[indexVal].second);
									nextStates.push_back(nextStateNum);
								}
							}
						}
						newControls.push_back(newControl);
						if (currentNum == 0) {
							CheckForReadiness(nextStates);
							nextPackagesStates.push_back(nextStates);
						}
					}
				}
			}
			//if (newControls.size()==0) cout << "newControls.size() == 0!" << endl;
			oneStateControls = newControls;
			if (currentNum == 0) nextPackageStateNumbers.push_back(nextPackagesStates);
		}
	}
	catch(const string msg){
		cout << msg << endl;
		ex << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

void Workflow::CheckForReadiness(vector<int>& nextStates){
	for (unsigned int i = 0; i < nextStates.size(); i++){
		if (!IsPackageInit(i) && nextStates[i]==0){
			bool allPredecessorsReady = true;
			for (unsigned int j = 0; j < connectMatrix.size(); j++){
				if (connectMatrix[j][i]==1 && nextStates[j]!=packages[j]->GetStatesCount()-1){
					allPredecessorsReady = false;
					break;
				}
			}
			if (allPredecessorsReady) nextStates[i]++;
		}
	}
}

bool Workflow::CheckCores(pair<int,int>& currentControl, vector <int>& otherControls){
	int initIndex = _refResources[currentControl.first-1]->GetInitVal();
	int resCoresCount = _refResources[currentControl.first-1]->GetCoresCount();
	int lastIndex = initIndex + resCoresCount - 1;
	int fullCoresCount = currentControl.second;
	for (unsigned int i = 0; i < otherControls.size(); i++){
		if (otherControls[i]!=-1){
		if (otherControls[i] >= initIndex && otherControls[i] <=lastIndex)
			fullCoresCount += otherControls[i]-initIndex + 1;
		}
	}
	if (fullCoresCount > resCoresCount) return false;
	return true;
}

bool Workflow::Check (const int& state, const vector <int> & otherStates, const int & currentPackage){
	double stateLevel = packages[currentPackage]->GetLevel(state);
	int core = packages[currentPackage]->GetCore(state);
	const int type = packages[currentPackage]->GetType(state);
	for (int i = otherStates.size()-1; i >=0; i--){
		const int &otherState = otherStates[i];
		const int & otherPackage = currentPackage + i + 1;
		double otherStateLevel = packages[otherPackage]->GetLevel(otherState);
		// if  otherPackage depends on currentPackage
		if (IsDepends(currentPackage, otherPackage)){
			if (stateLevel!=1 && otherStateLevel!=-1) return false;
			if (stateLevel==1 && otherStateLevel==-1){
				bool isDependsOnPrevious = false;
				// if package depends on packages that will be considered later, we miss this state
				for (int j = 0; j < currentPackage; j++){
					if (IsDepends(j,otherPackage)) {
						isDependsOnPrevious = true;
						break;
					}
				}
				if (isDependsOnPrevious) continue;
				bool isOtherReady = true;
				for (unsigned int j = 0; j < otherStates.size(); j++){
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

void Workflow::PrintPackagesStates(vector <vector<int>> &packagesStates){
	string f = "wf"+to_string((long long)wfNum) + ".txt";
	ofstream file (f);
	for (unsigned int i = 0; i < packagesStates.size(); i++){
		for (unsigned int j = 0; j < packagesStates[i].size(); j++){
			packages[j]->PrintState(file, packagesStates[i][j]);
			file << " ";
		}
		file << endl;
	}
	file.close();
}

void Workflow::PrintControls(vector <vector<int>> &packagesStates, vector <vector<vector<int>>>& controls, 
	vector <vector<int>>&nextStateNumbers){
	string fileName = "wf" + to_string((long long)wfNum) + "_controls.txt";
	ofstream f(fileName);
	for (unsigned int i = 0; i < controls.size(); i++){
		f << "State " << i+1 << endl;
		for (unsigned int j = 0; j < packagesStates[i].size(); j++){
			f << packagesStates[i][j];
			f << " ";
		}
		// print state
		/*for (unsigned int j = 0; j < packagesStates[i].size(); j++){
			packages[j]->PrintState(f, packagesStates[i][j]);
			f << " ";
		}*/
		f << endl;
		
		for (unsigned int j = 0; j < controls[i].size(); j++){
			for (unsigned int k = 0; k < controls[i][j].size(); k++){
				f << controls[i][j][k] << " ";
			}

			/*for (unsigned int k = 0; k < controls[i][j].size(); k++){
				int control = controls[i][j][k];
				if (control == -1) f << "(0 0)";
				else f << "(" << _reftypesCores[control].first << " " << _reftypesCores[control].second << ")";
			}*/
			f << endl;
			f << nextStateNumbers[i][j] << " ";
			/*for (unsigned int k = 0; k < packagesStates[nextStateNumbers[i][j]].size(); k++){
				
				packages[k]->PrintState(f, packagesStates[nextStateNumbers[i][j]][k]);
				f << " ";
			}*/
			f << endl;
		}

	}
	f.close();
}

void Workflow::PrintState(const vector <int>& state, ofstream &f){
	vector<int>::const_iterator stateIt = state.begin();
	int packageIndex = 0;
	for (; stateIt!=state.end() ; stateIt++){
		packages[packageIndex]->PrintState(f,state[packageIndex]);
		f << " ";
		packageIndex++;
	}
	f << endl;
}

void Workflow::PrintControl(const vector <int>& control, ofstream &f){
	vector<int>::const_iterator controlIt = control.begin();
	for (; controlIt!=control.end() ; controlIt++){
		if (*controlIt == -1) f << "(0 0)";
		else f << "(" << _reftypesCores[*controlIt].first << " " << _reftypesCores[*controlIt].second << ")";
		f << " ";
	}
	f << endl;
}

void Workflow::PrintExecTime(){
	string name = "wf" + to_string((long long)wfNum) + "_execTimes.txt";
	ofstream f(name);
	for (vector<Package*>::size_type i = 0; i < packages.size(); i++ ){
		packages[i]->PrintExecTime(f);
	}
	f.close();
}

bool Workflow::IsDepends(unsigned int one, unsigned int two){
	try {
		string errorMsg = "IsDepends() error (workflow " + to_string((long long) wfNum) + "): incorrect package num - ";
		if (one > packages.size()-1) throw errorMsg +  to_string((long long) one);
		if (two > packages.size()-1) throw errorMsg +  to_string((long long) two);
		if (connectMatrix[one][two]==1) return true;
		else {
			vector <int> notDirectDepends;
			for (unsigned int i = 0; i < connectMatrix[one].size(); i++){
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
	for (unsigned int i = 0; i < connectMatrix[pNum].size(); i++){
		if (connectMatrix[i][pNum]==1) res.push_back(i);
	}
}

bool Workflow::IsPackageInit(int pNum){
	bool flag = true;
	for (unsigned int i = 0; i < connectMatrix.size(); i++){
		if (connectMatrix[i][pNum]!=0) return false;
	}
	return flag;
}

void Workflow::GetExecTime(map <pair <int,int>, double> & out, int pNum){
	try{
		if (pNum < 0 || pNum > packages.size()-1)
			throw UserException("Workflow::GetExecTime() : wrong pNum");
		packages[pNum]->GetExecTime(out);
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

/*
parameters:
state - vector of packages states
timeCoresPerType - out parameter
packagesIndexesPerType - out parameter, array of types size which contains indexes of packages (from 0) executing on some type
*/
void Workflow::SetTimesCoresForState(const vector<int>&state, vector<vector<pair<double, unsigned int>>>&timeCoresPerType,
	vector<vector<int>>& packagesIndexesPerType){
	try{
		vector <int>::const_iterator stateIt = state.begin();
		unsigned int packageIndex = 0;
		for (; stateIt!=state.end(); stateIt++){
			unsigned int type = packages[packageIndex]->GetType(*stateIt);
			if (type!=0){
				timeCoresPerType[type-1].push_back(make_pair(packages[packageIndex]->GetExecTime(*stateIt),
					packages[packageIndex]->GetCore(*stateIt)));
				packagesIndexesPerType[type-1].push_back(packageIndex);
			}
			packageIndex++;
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Workflow::SetTimesCoresForControl(const vector<int>&state, const vector<int>&control, 
	vector<vector<pair<double, unsigned int>>>&timeCoresPerType, vector<vector<int>>& packagesIndexesPerType){
	vector <int>::const_iterator controlIt = control.begin();
	int packageIndex = 0;
	for (;controlIt!=control.end(); controlIt++){
		if (*controlIt!=-1){
			int type = _reftypesCores[*controlIt].first;
			int core = _reftypesCores[*controlIt].second;
			// if control isn`t "zero" and level is null
			if (type != 0 && packages[packageIndex]->GetLevel(state[packageIndex])==0) {
				timeCoresPerType[type-1].push_back(make_pair(packages[packageIndex]->GetExecTime(type,core), core));
				packagesIndexesPerType[type-1].push_back(packageIndex);
			}
		}
		packageIndex++;
	}
}

double Workflow::GetLevel(int pNum, int state){
	return packages[pNum]->GetLevel(state);
}

double Workflow::GetExecTime(int pNum, int state){
	return packages[pNum]->GetExecTime(state);
}

double Workflow::GetExecTime(int pNum, int type, int cores) {
	try{
		if (pNum < 0 || pNum > packages.size()-1) throw UserException("Workflow::GetExecTime() : wrong packageNum " + to_string((long long)pNum));
		return packages[pNum]->GetExecTime(type, cores);
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}