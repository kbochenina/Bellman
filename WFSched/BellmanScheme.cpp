#include "StdAfx.h"
#include "BellmanScheme.h"
#include "UserException.h"
#include <ctime>
#include <iostream>
#include <fstream> // ReadData()
#include <sstream> // ReadData()

vector <int> BellmanScheme::settedWFs;

BellmanScheme::BellmanScheme(DataInfo &d,int u, int w) : SchedulingMethod(d,u,w){ 
	std::cout << "BellmanScheme is instantiated\n";
	// if initial data for WF was already setted,
	// read data from file
	// else initialize the data ;-(
	bool wasSetted = false;
	for (const auto &ptr: settedWFs){
		if (ptr == wfNum) {
			wasSetted = true;
			break;
		}
	}
	if (wasSetted){
		int t = clock();
		ReadData();
		cout << "Time of reading data = " << (clock() - t)/1000.0 << endl;
	}
	else SetData();
}

void BellmanScheme::printInfo(){
	//std::cout << "BellmanScheme is instantiated\n";
	//std::cout << "koeff = " << eff->GetKoeff() << "\n";
}

double BellmanScheme::GetWFSchedule(Schedule &out){
	
	return 0.0;
}

void BellmanScheme::SetData(){
	SetPackagesStates();
	int t = clock();
	// setting states, controls, nextStates
	SetStates(0);
	cout << "Time of setting states = " << (clock() - t)/1000.0 << endl;
	t = clock();
	SetControls();
	cout << "Time of setting controls and next states = " << (clock() - t)/1000.0 << endl;
	t = clock();
	PrintData();
	//Workflows[i]->SetFullPackagesStates(0, states, controls, nextStateNumbers);
	//for (int j = 0; j < stages; j++) 
	//	FullInfo[j].resize(states.size());
	////cout << "Time of SetFullPackagesStates() " << (clock()-t)/1000.0 << endl;
	////t = clock();
	//Workflows[i]->PrintPackagesStates(states);
	////cout << "Time of PrintPackagesStates() " << (clock()-t)/1000.0 << endl;
	//Workflows[i]->PrintControls(states,controls, nextStateNumbers);
	////cout << "Time of PrintControls() " << (clock()-t)/1000.0 << endl;
	//states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
	settedWFs.push_back(wfNum);
}

void BellmanScheme::PrintData(){
	string fileName = "wf" + to_string(wfNum) + "_controls.txt";
	ofstream f(fileName);
	for (unsigned int i = 0; i < controls.size(); i++){
		f << "State " << i+1 << endl;
		for (unsigned int j = 0; j < states[i].size(); j++){
			f << states[i][j];
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
			f << nextStates[i][j] << " ";
			/*for (unsigned int k = 0; k < packagesStates[nextStateNumbers[i][j]].size(); k++){
				
				packages[k]->PrintState(f, packagesStates[nextStateNumbers[i][j]][k]);
				f << " ";
			}*/
			f << endl;
		}

	}
	PrintStates();
	f.close();
}

// print states info to file 
void BellmanScheme::PrintStates(){
	string fileName = "wf" + to_string(wfNum) + "_states.txt";
	ofstream f(fileName);
	for (auto package: pStates)
		package.PrintStates(f);
	f.close();
}

void BellmanScheme::ReadData(){
	try{
		if (wfNum < 0) 
			throw UserException("BellmanScheme::ReadData() error. Negative wfNum");
		int packages = data.Workflows(wfNum).GetPackageCount();
		string name = "wf" + to_string(wfNum) + "_controls.txt";
		ifstream f(name);
		if (f.fail()) 
			throw UserException("BellmanScheme::ReadData() error. File " + name + "cannot be open");
		int line = 0;
		string errEarlyEnd = "BellmanScheme::ReadData() error. Early file end";
		string s;
		getline(f,s);
		line++;
		if (f.eof()) 
			throw UserException(errEarlyEnd);
		if (s.find("State")==string::npos) 
			throw UserException("BellmanScheme::ReadData() error. Wrong format at line " + to_string(line));
		while (!f.eof()){
			// read state
			getline(f,s);
			line++;
			if (f.eof()) 
				throw UserException(errEarlyEnd);
			istringstream iss(s);
			vector <int> state;
			for (int i = 0; i < packages; i++){
				iss >> s;
				state.push_back(stoi(s));
			}
			states.push_back(move(state));
			getline(f,s);
			line++;
			if (f.eof()) 
				throw UserException(errEarlyEnd);
			// reading controls and next states
			vector <vector<int>> oneStateControls;
			vector<int> nextStates;
			do 
			{
				iss.str(s);
				iss.clear();
				vector <int> control;
				for (int i = 0; i < packages; i++){
					iss >> s;
					control.push_back(stoi(s));
				}
				oneStateControls.push_back(move(control));

				getline(f,s);
				line++;
				if (f.eof()) throw UserException(errEarlyEnd);
				nextStates.push_back(stoi(s));

				getline(f,s);
				line++;

			} while (s.find("State")==string::npos && !f.eof());
			controls.push_back(move(oneStateControls));
			this->nextStates.push_back(move(nextStates));
		}
		ReadStates();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
	catch (std::exception& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void BellmanScheme::ReadStates(){
	try{
		if (wfNum < 0) 
			throw UserException("BellmanScheme::ReadStates() error. Negative wfNum");
		int packages = data.Workflows(wfNum).GetPackageCount();
		string name = "wf" + to_string(wfNum) + "_states.txt";
		ifstream f(name);
		if (f.fail()) 
			throw UserException("BellmanScheme::ReadStates() error. File " + name + "cannot be open");
		int line = 0;
		string errEarlyEnd = "BellmanScheme::ReadStates() error. Early file end";
		string s;
		for (int i = 0; i < packages; i++){
			// Package #
			getline(f,s);
			line++;
			if (f.eof()) 
				throw UserException(errEarlyEnd);
			if (s.find("Package")==string::npos) 
				throw UserException("BellmanScheme::ReadStates() error. Wrong format at line " + to_string(line));
			getline(f,s);
			line++;
			if (f.eof()) 
				throw UserException(errEarlyEnd);
			int stateNum = 0;
			int type, core;
			double level;
			vector <PackageState> packageStates;
			string search = "State # ";
			istringstream iss(s);
			// State #
			while (s.find(search) != string::npos){
				s.erase(0, search.length());
				iss.str(s);
				iss >> stateNum;
				iss >> type >> core >> level;
				packageStates.push_back(make_tuple(type, core, level));
				getline(f,s);
			}
			PackageStates newP(i,packageStates);
			pStates.push_back(move(newP));
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
	catch (std::exception& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

// setting packages states
void BellmanScheme::SetPackagesStates(){
	const Workflow &w = data.Workflows(wfNum);
	// for all packages
	for (auto uid = 0; uid < w.GetPackageCount(); uid++){
	try {
		vector <PackageState> packageStates;
		if (!w.IsPackageInit(uid)) 
			packageStates.push_back(make_tuple(0, 0, -1));
		packageStates.push_back(make_tuple(0, 0, 0));
		// get ((type, core), (execTime)) vector
		const times& execTimes = w[uid].GetExecTimes();
		// for each configuration
		for (auto & time: execTimes){
			double execTime = time.second;
			double f = 1.00/((int)execTime/data.GetDelta() + 1);
			for (double k = f; k < 1; k+= f)
				packageStates.push_back(make_tuple(time.first.first, time.first.second, k));
		}

		packageStates.push_back(make_tuple(0, 0, 1));
		PackageStates obj(uid, packageStates);
		pStates.push_back(move(obj));
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
	printInfo();
	}
}

void BellmanScheme::SetStates(int currentPackage){
	const Workflow &w = data.Workflows(wfNum);
	int packagesCount = w.GetPackageCount();
	// if it is the last package
	// just push to the states vector
	// numbers of its states
	if (currentPackage == packagesCount-1) {
		vector <int> lastPackageStates;
		for (int i = 0; i < pStates[currentPackage].GetStatesCount(); i++) {
			lastPackageStates.push_back(i);
			states.push_back(lastPackageStates);
			lastPackageStates.clear();
		}
	}
	// if it is not the last package
	else {
		vector <int> currentPackageStates;
		vector <vector <int>> newStates;
		vector <int> newState;
		newState.resize(packagesCount - currentPackage);
		for (int i = 0; i < pStates[currentPackage].GetStatesCount(); i++) 
			currentPackageStates.push_back(i);
		SetStates(currentPackage+1);
	
		for (const auto& i : currentPackageStates){
			for (unsigned int j = 0; j < states.size(); j++){
				if (CheckState(currentPackageStates[i], states[j], currentPackage)) {
					newState[0] = currentPackageStates[i];
					for (unsigned int k = 1; k <= states[j].size(); k++)
						newState[k] = states[j][k-1];
					newStates.push_back(newState);
				/*	if (currentPackage == 0){
						int ct = clock();
						GetControls(newState, 0); 
						tf << "Time of getting controls " << (clock()-ct)/1000.0 << endl;
						fctime+=(clock()-ct)/1000.0;
						tf << "oneStateControls.size() " << oneStateControls.size() << endl;
						controls.push_back(oneStateControls);
					}*/
				}
			}
		}
			
		states = std::move(newStates);
		/*if (currentPackage == 0) 
			SetNextStateNumbers(packagesStates, controls, nextStateNumbers);*/
	}
}

// check the state for consistency
// (dependancy requirements and full core count)
// (otherStates is already checked)
bool BellmanScheme::CheckState(const int& state, const vector <int>& otherStates, int currentPackage){
	const Workflow &w = data.Workflows(wfNum);
	double level = pStates[currentPackage].GetLevel(state);
	int core = pStates[currentPackage].GetCore(state);
	int type = pStates[currentPackage].GetType(state);
	for (int i = otherStates.size()-1; i >=0; i--){
		int otherState = otherStates[i];
		int otherPackage = currentPackage + i + 1;
		double otherStateLevel = pStates[otherPackage].GetLevel(otherState);
		// if  otherPackage depends on currentPackage
		if (w.IsDepends(currentPackage, otherPackage)){
			if (level!=1 && otherStateLevel!=-1) return false;
			if (level==1 && otherStateLevel==-1){
				bool isDependsOnPrevious = false;
				// if package depends on packages that will be considered later, we miss this state
				for (int j = 0; j < currentPackage; j++){
					if (w.IsDepends(j,otherPackage)) {
						isDependsOnPrevious = true;
						break;
					}
				}
				if (isDependsOnPrevious) continue;
				bool isOtherReady = true;
				for (unsigned int j = 0; j < otherStates.size(); j++){
					if ( i != j && w.IsDepends(currentPackage+j+1, otherPackage) 
						&& pStates[currentPackage+j+1].GetLevel(otherStates[j]) != 1)
						isOtherReady = false;
				}
				if (isOtherReady) return false;
			}
		}
		else {
			if (type){
				const int otherType = pStates[otherPackage].GetType(otherState);
				if (type == otherType){
					core += pStates[otherPackage].GetCore(otherState);
				}
			}
		}
	}
	// if we haven't enough resources, state is not legal
	if (type && core > data.Resources(type-1).GetCoresCount()) 
		return false;
	return true;
}

// procedure for getting controls
void BellmanScheme::SetControls(){
	// for each state
	for (auto state: states){
		GetControls(state, 0);
	}
}

// should exist between GetControls() executions
vector<vector<int>> oneStateControls;
// for each state:  for each control - packages next state numbers
// for example:
// state 1, (package 1 - #5),(package 2 - # 1), (package 3 - #0) ...
// state 2, ...
vector <vector<vector<int>>> nextPackageStateNumbers; 

// recursive function for getting controls for one state
void BellmanScheme::GetControls(const vector<int>& state, int currentPackage){
	const Workflow &w = data.Workflows(wfNum);
	// index of (type, core) in typesCores array for each package
	vector <int> currentControl;
	// (type, core)
	pair<int,int> typeCore;
	// index in typeCores
	int typeCoreIndex = 0;
	int stateNumber = state[currentPackage];
	double level = pStates[currentPackage].GetLevel(stateNumber);
	// if the package cannot be started
	// or it has been already executed 
	// the control can be only (-1, -1) - nothing to do
	if (level == -1 || level == 1) {
		currentControl.push_back(-1);
	}
	// if the package is executing now, 
	// its control cannot change
	else if (level > 0){
		typeCore = make_pair(pStates[currentPackage].GetType(stateNumber), 
				pStates[currentPackage].GetCore(stateNumber));
		typeCoreIndex = data.GetTypeCoreIndex(typeCore);
		currentControl.push_back(typeCoreIndex);
	}
	else {
		vector <int> r = w[currentPackage].GetResTypes(), 
			c = w[currentPackage].GetCoreCounts();

		// if package can be started, we can also delay its start
		currentControl.push_back(-1);
		// check possible combinations of types and cores
		for (unsigned int i = 0; i < r.size(); i++){
			for (unsigned int j = 0; j < c.size(); j++){
				// if resourceType[r[i]-1] has enough cores
				if (c[j] <= data.Resources(r[i]-1).GetCoresCount()){
					typeCore = make_pair(r[i], c[j]);
					typeCoreIndex = data.GetTypeCoreIndex(typeCore);
					currentControl.push_back(typeCoreIndex);
				}
			}
		}
	}
	// if it is the last package	
	if (currentPackage == w.GetPackageCount()-1){
		for (const auto& i : currentControl) {
			// pack currentControl[i] in a vector
			vector <int> one;
			one.push_back(i);
			oneStateControls.push_back(move(one));
		}
	}
	else {
		GetControls(state, currentPackage+1);
		// check for compatibility
		vector <vector <int>> newControls;
		vector <vector <int>> nextPackagesStates;
		for (const auto &i : currentControl){
			for (unsigned int j = 0; j < oneStateControls.size(); j++){
				vector<int> newControl;
				vector <int> nextStates;
				// if currentControl doesn't use resources 
				// or we have enough resources for currentControl[i]+oneStateControls[j]
				if (i == -1 || CheckCores(i, oneStateControls[j])){
					// first element is currentControl[i]
					newControl.push_back(i);
					// if we should construct nextState
					if (currentPackage == 0) {
						if (i == -1) 
							// state is not changed
							nextStates.push_back(state[currentPackage]);
						else {
							// get next state by current state and control (type, core)
							int nextStateNum = pStates[currentPackage].GetNextStateNum(state[currentPackage], 
								data.TypesCores(i));
							nextStates.push_back(nextStateNum);
						}
					}
					// add other values to newControl
					for (unsigned k = 0; k < oneStateControls[j].size(); k++) {
						typeCoreIndex = oneStateControls[j][k];
						newControl.push_back(typeCoreIndex);
						if (currentPackage == 0){
							// add other values to nextStates
							if (typeCoreIndex == -1) 
								nextStates.push_back(state[k+1]);
							else {
								int nextStateNum = pStates[k+1].GetNextStateNum(state[k+1], 
									data.TypesCores(typeCoreIndex));
								nextStates.push_back(nextStateNum);
							}
						}
					}
					newControls.push_back(move(newControl));
					if (currentPackage == 0) {
						CheckForReadiness(nextStates);
						nextPackagesStates.push_back(move(nextStates));
					}
				}
			}
		}
		oneStateControls = move(newControls);
		if (currentPackage == 0) {
			nextPackageStateNumbers.push_back(nextPackagesStates);
			SetNextStateNumbers();
			controls.push_back(move(oneStateControls));
			nextPackageStateNumbers.clear();
			oneStateControls.clear();
		}
	}
}

// find next state numbers in states according to next packages states
// additional function for GetControls
// very long function
void BellmanScheme::SetNextStateNumbers(){
	try{
		// i is also stateNumber
		for (unsigned int i = 0; i < nextPackageStateNumbers.size(); i++){
			vector <int> readyNumbers;
			for (unsigned int j = 0; j < nextPackageStateNumbers[i].size(); j++){
				vector<vector<int>>::iterator it = find(states.begin()+i, states.end(), nextPackageStateNumbers[i][j]);
				if (it==states.end()) 
					throw UserException("SetNextStateNumbers() error: cannot find next state");
				// find next state number
				readyNumbers.push_back(distance(states.begin(),it));
				/*for (int stateIndex = i; stateIndex < states.size(); stateIndex++){
					bool flag = true;
					for (int k = 0; k < nextPackageStateNumbers[i][j].size(); k++){
						if (nextPackageStateNumbers[i][j][k] != states[stateIndex][k]) {
							flag = false; break;
						}
					}
					if (flag) { 
						readyNumbers.push_back(stateIndex); break;
					}
				}*/
			}
		
			nextStates.push_back(move(readyNumbers));
		}

	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

// if all predecessors of package are ready,
// package can start execution
// we should check it after producing next states
void BellmanScheme::CheckForReadiness(vector<int>& nextStates){
	for (unsigned i = 0; i < nextStates.size(); i++){
		// if package state is equal to (0, 0, -1)
		if (!data.Workflows(wfNum).IsPackageInit(i) && nextStates[i]==0){
			bool allPredecessorsReady = true;
			for (auto j = 0; j <  data.Workflows(wfNum).GetPackageCount(); j++){
				// if j is a predecessor of i and j level != 1
				// then task i cannot start execution
				if (data.Workflows(wfNum).GetMatrixValue(j,i) == 1 
					&& nextStates[j]!= pStates[j].GetStatesCount()-1){
					allPredecessorsReady = false;
					break;
				}
			}
			// if all predecessors are ready
			// then we move state from (0, 0, -1) to (0, 0, 0) [ready to execution]
			if (allPredecessorsReady) nextStates[i]++;
		}
	}
}


bool BellmanScheme::CheckCores(const int& currentControl, const vector <int>& otherControls){
	// reference to typesCores from data
	const vector<pair<int,int>>& typesCores =  data.TypesCores();
	// index of resource
	int resIndex = typesCores[currentControl].first;
	// resource cores count
	int resCoresCount = data.Resources(resIndex-1).GetCoresCount();
	int fullCoresCount = typesCores[currentControl].second;
	for (unsigned int i = 0; i < otherControls.size(); i++){
		// if i-th package is executed on resource # resIndex
		// add its coresCount to fullCoresCount
		if (otherControls[i]!=-1 && typesCores[otherControls[i]].first == resIndex){
			fullCoresCount += typesCores[otherControls[i]].second;
		}
	}
	if (fullCoresCount > resCoresCount) return false;
	return true;
}

BellmanScheme::~BellmanScheme(void)
{
}
