#include "StdAfx.h"
#include "Model.h"




//bool mycompare (const OnePackageControl &lhs, const OnePackageControl &rhs){
//	return get<0>(lhs) < get<0>(rhs);
//}
//
//bool packageStateOrder (const PackageState &lhs, const PackageState &rhs){
//	return get<0>(lhs) < get<0>(rhs);
//}

bool SortPairs(pair <int, double> p1, pair <int,double> p2){
	return (p1.second > p2.second);
}

void Model::Init (string resFile, string wfFile, string settingsFile){
	InitResources(resFile); cout << "Initialization of resources ended" << endl;
	FullInfo.resize(stages);
	InitWorkflows(wfFile); 
	InitSettings(settingsFile);
}

void Model::InitSettings(string settingsFile){
	try{
		char second[21]; 
		ifstream file(settingsFile, ifstream::in);
		string errOpen = "File " + settingsFile + " was not open";
		string errWrongFormat = "Wrong format in file " + settingsFile + " at line ";
		string errWrongValue = "Wrong value of parameter ";
		string errWrongFormatFull = errWrongFormat;
		if (file.fail()) throw errOpen;
		unsigned int line = 0;
		string s, trim;
		getline(file,s);
		++line;
		trim = "canExecuteOnDiffResources=";
		size_t found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw errWrongFormatFull;
		}
		s.erase(0,trim.size());
		int flag = atoi(s.c_str());
		if (flag==1) canExecuteOnDiffResources = true;
		else if (flag == 0) canExecuteOnDiffResources = false;
		else throw errWrongValue + "canExecuteOnDiffResources";
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

void Model::InitWorkflows(string f){
	try{
		char second[21]; // enough to hold all numbers up to 64-bits
		ifstream file(f, ifstream::in);
		string errOpen = "File " + f + " was not open";
		string errEarlyEnd = "Unexpected end of file " + f;
		string errWrongFormat = "Wrong format in file " + f + " at line ";
		string errPackagesCount = "Packages count cannot be less than 1";
		string errCoresCount = "Cores count cannot be less than 1";
		string errConnMatrix = "Wrong value in connectivity matrix";
		string errWrongFormatFull = errWrongFormat;
		if (file.fail()) throw errOpen;

		string s, trim; int line = 0;
		getline(file,s);
		++line;
		if (file.eof()) throw errEarlyEnd;
		trim = "Workflows count = ";
		size_t found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw errWrongFormatFull;
		}
		s.erase(0,trim.size());
		int workflowsCount = atoi(s.c_str());
		vector <int> types; map <pair <int,int>, double> execTime; vector <Package*> pacs; 
		vector <int> cCount; 
		vector <vector <int>> connectMatrix;
		int fullPackagesCount = 0;
		for (int i = 0; i < workflowsCount; i++){
			int packagesCount = 0;
			getline(file,s);
			++line;
			if (file.eof()) throw errEarlyEnd;
			
			if ((found = s.find("(")) == std::string::npos){
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			s.erase(0,found+1);
			istringstream iss(s);
			iss >> packagesCount;
			if (iss.fail()) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			if (packagesCount < 1) {
				sprintf_s(second, "%d", i+1);
				string beginStr = "Workflow ";
				beginStr += second;
				beginStr += " - ";
				beginStr += errPackagesCount;
				throw beginStr;
			}
			for (int j = 0; j < packagesCount; j++){
				double alpha = 0.0; // part of consequentually executed code
				++fullPackagesCount;
				// Package [packageNumber]
				getline(file,s);
				if (file.eof()) throw errEarlyEnd;
				++line;
				// Alpha: [alpha value]
				getline(file,s);
				if (file.eof()) throw errEarlyEnd;
				++line;
				trim = "Alpha: ";
				size_t found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				iss >> alpha;
				if (iss.fail()) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				// Resource types: [resource types values]. -1 means all possible resources
				getline(file,s);
				if (file.eof()) throw errEarlyEnd;
				++line;
				trim = "Resources types: ";
				found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				int typeNumber = 0;
				string comma;
				do{
					comma = "";
					iss >> typeNumber;
					// if package can execute on all possible resources
					if (typeNumber == -1){
						for (unsigned int i = 0; i < Resources.size(); i++)
							types.push_back(i+1);
						break;
					}
					if (iss.fail()) {
						sprintf_s(second, "%d", line);
						errWrongFormatFull += second;
						throw errWrongFormatFull;
					}
					types.push_back(typeNumber);
					iss >> comma;
				} while (comma==",");


				
				// Cores count: [cores count values]
				getline(file,s);
				if (file.eof()) throw errEarlyEnd;
				++line;
				trim = "Cores count: ";
				found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				s.erase(0,trim.size());
				int coresCount = 0;
				iss.str(s);
				iss.clear();
				int coreCount = 0;
				do{
					comma = "";
					iss >> coreCount;
					if (iss.fail()) {
						sprintf_s(second, "%d", line);
						errWrongFormatFull += second;
						throw errWrongFormatFull;
					}
					cCount.push_back(coreCount);
					iss >> comma;
				} while (comma==",");
				
				if (cCount.size() < 1) {
					sprintf_s(second, "%d", i+1);
					string beginStr = "Workflow ";
					beginStr += second;
					beginStr += " - ";
					beginStr += errCoresCount;
					throw beginStr;
				}
				
				// Computational amount: [amount value]
				long int amount = 0;
				getline(file,s);
				if (file.eof()) throw errEarlyEnd;
				++line;
				trim = "Computation amount: ";
				found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				iss >> amount;
				if (iss.fail()) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}

				for (unsigned int k = 0; k < types.size(); k++){
					for (unsigned int l = 0; l < cCount.size(); l++){
						// assume that the core numbers are in ascending order (else continue)
						if (Resources[k]->GetCoresCount() < cCount[l]) break; 
						// Amdal's law
						double acc = (double) 1.00 / (alpha + (1-alpha)/(l+1));
						// execTime = amount / (perf * acc)
						double exTime = amount / (Resources[k]->GetPerf() * acc);
						execTime.insert(make_pair(make_pair(types[k], cCount[l]), exTime));
					}
				}
												
				pacs.push_back(new Package(fullPackagesCount,types,cCount,execTime));
				types.clear();
				execTime.clear();
				cCount.clear();
			}
			getline(file,s);
			if (file.eof()) throw errEarlyEnd;
			++line;
			for (int j = 0; j < packagesCount; j++){
				vector <int> row;
				getline(file,s);
				if (file.eof()) throw errEarlyEnd;
				++line;
				iss.str(s);
				iss.clear();
				for (int k = 0; k < packagesCount; k++){
					int val = 0;
					iss >> val;
					if (iss.fail()) {
						sprintf_s(second, "%d", line);
						errWrongFormatFull += second;
						throw errWrongFormatFull;
					}
					if (val!=0  && val!=1){
						sprintf_s(second, "%d", i+1);
						string beginStr = "Workflow ";
						beginStr += second;
						beginStr += " - ";
						beginStr += errConnMatrix;
						throw beginStr;
					}
					row.push_back(val);
				}
				connectMatrix.push_back(row);
			}
			
			Workflows.push_back(new Workflow(pacs,connectMatrix,i+1, Resources, typesCores));
			pacs.clear();
			connectMatrix.clear();
		}
		for (unsigned int i = 0; i < Workflows.size(); i++){
			Workflows[i]->SetIsPackageInit();
			Workflows[i]->SetPackagesStates();
			Workflows[i]->PrintExecTime();
			int t = clock();
			Workflows[i]->SetFullPackagesStates(0, states, controls, nextStateNumbers);
			for (int j = 0; j < stages; j++) 
				FullInfo[j].resize(states.size());
			cout << "Time of SetFullPackagesStates() " << (clock()-t)/1000.0 << endl;
			t = clock();
			Workflows[i]->PrintPackagesStates(states);
			cout << "Time of PrintPackagesStates() " << (clock()-t)/1000.0 << endl;
			Workflows[i]->PrintControls(states,controls, nextStateNumbers);
			cout << "Time of PrintControls() " << (clock()-t)/1000.0 << endl;
		}
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	
}

void Model::InitResources(string f){
	try{
		map <int, vector<pair <int,int>>> busyIntervals;
		char second[21]; // enough to hold all numbers up to 64-bits
		ifstream file(f.c_str(), ifstream::in);
		string errOpen = "File " + f + " was not open";
		string errEarlyEnd = "Unexpected end of file " + f;
		string errWrongFormat = "Wrong format in file " + f + " at line ";
		string errWrongFormatFull = errWrongFormat;
		if (file.fail()) 
			throw errOpen;
		string s, trim; int line = 0;
		getline(file,s);
		++line;
		if (file.eof()) throw errEarlyEnd;
		trim = "Resources count = ";
		size_t found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw errWrongFormatFull;
		}
		s.erase(0,trim.size());
		int allResourcesCount = atoi(s.c_str());

		trim = "Resources types count = ";
		getline(file,s);
		++line;
		if (file.eof()) throw errEarlyEnd;
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw errWrongFormatFull;
		}
		s.erase(0,trim.size());
		int typesCount = atoi(s.c_str());
		int resourcesCount, coresCount = 0;
		
		for (int i = 0; i < typesCount; i++)
		{
			vector <Resource> typeResources;
			istringstream iss(s);
			getline(file,s);
			++line;
			if (file.eof()) throw errEarlyEnd;
			sprintf_s(second, "%d", i+1);
			string first = "Type ";
			trim = first + second;
			found = s.find(trim);
			if (found != 0) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			s.erase(0,trim.size()+2);
			iss.str(s);
			iss.clear();
			iss >> resourcesCount;
			if (iss.fail()) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			found = s.find(",");
			s.erase(0,found+2);
			iss.str(s);
			iss.clear();
			iss >> coresCount;
			if (iss.fail()) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			double perf = 0.0;
			getline(file,s);
			trim = "Performance (GFlops): ";
			found = s.find(trim);
			if (found != 0) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			s.erase(0,trim.size());
			perf = atof(s.c_str());

			for (int j = 0; j < resourcesCount; j++){
				getline(file,s);
				++line;
				if (file.eof()) throw errEarlyEnd;
				busyIntervals.clear();
				for (int k = 0; k < coresCount; k++){
					getline(file,s);
					++line;
					if (file.eof()) throw errEarlyEnd;
					sprintf_s(second, "%d", k+1);
					first = "Core ";
					trim = first + second;
					found = s.find(trim);
					if (found != 0) {
						sprintf_s(second, "%d", line);
						errWrongFormatFull += second;
						throw errWrongFormatFull;
					}
					s.erase(0,trim.size()+1);
					int diapCount = atoi(s.c_str());
					vector<pair<int,int>> oneResDiaps;
					for (int l = 0; l < diapCount; l++){
						if (file.eof()) throw errEarlyEnd;
						getline(file,s);
						++line;
						iss.str(s);
						iss.clear();
						int one,two;
						iss >> one;
						if (iss.fail()) {
							sprintf_s(second, "%d", line);
							errWrongFormatFull += second;
							throw errWrongFormatFull;
						}
						iss >> two;
						if (iss.fail()) {
							sprintf_s(second, "%d", line);
							errWrongFormatFull += second;
							throw errWrongFormatFull;
						}
						oneResDiaps.push_back(make_pair(one,two));
					}
					busyIntervals.insert(make_pair(k+1, oneResDiaps));
				}
				Resource oneRes(j+1,  coresCount, busyIntervals);
				
				typeResources.push_back(oneRes);
			}
			Resources.push_back(new ResourceType(i+1, typeResources, perf));
			
		}
		
		int initVal = 0;
		for (unsigned int i = 0; i < Resources.size(); i++) {
			Resources[i]->CorrectBusyIntervals(stageBorders);
			unsigned short fullCoresCount = Resources[i]->GetCoresCount();
			for (int j = 0; j < fullCoresCount; j++) typesCores.push_back(make_pair(i+1, j+1));
			Resources[i]->SetInitLastVals(initVal, initVal+fullCoresCount - 1);
			initVal +=fullCoresCount;
		}
		SetForcedBricks();
		int allCoresCount = 0;
		for (unsigned int i = 0; i < Resources.size(); i++) {
			Resources[i]->SetFreeTimeEnds();
			Resources[i]->SetChildForcedBricks();
			allCoresCount += Resources[i]->GetCoresCount();
		}
		koeff = 2.00 / allCoresCount;
		file.close();
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	
	
}

void Model::SetForcedBricks(){
	vector <int>* currentVec; 
	int beginVal = 0;
	for (unsigned int i = 0; i < Resources.size(); i++) {
		vector <int> resourceTypeForcedBricks;
		for (int j = 0; j < Resources[i]->GetResourceCount(); j++){
		currentVec = (*Resources[i])[j].GetForcedNumbers();
		//cout << "Current vec: ";
		for (unsigned int k = 0; k < currentVec->size(); k++) {
			//cout << (*currentVec)[j] << " ";
			(*currentVec)[k]+=beginVal;
		}
		//cout << endl;
		copy(currentVec->begin(), currentVec->end(), back_inserter(forcedBricks));
		copy(currentVec->begin(), currentVec->end(), back_inserter(resourceTypeForcedBricks));
		//cout << "Forced brics: ";
		//for (int j = 0; j < forcedBrics.size(); j++) cout << forcedBrics[j] << " ";
		//cout << endl;
		beginVal += (*Resources[i])[j].GetCoresCount() * T / delta;
		}
		Resources[i]->SetForcedBricks(resourceTypeForcedBricks);
	}
}





void Model::DirectBellman(){
	try{
		ofstream f("result.txt");
		int currentNum = 0;
		double maxEff = FullInfo[0][0].second;
		for (int i = 0; i < stages; i++){
			vector <vector<int>> fullUsedNums;
			timeCore timeCores;
			timeCores.resize(Resources.size());
			int uopt = FullInfo[i][currentNum].first;
			Workflows[0]->PrintState(states[currentNum], f);
			Workflows[0]->PrintControl(controls[currentNum][uopt], f);
			CheckControl(currentNum,uopt,i,timeCores, fullUsedNums, true);
			int fullUsedNumIndex = 0;
			string errMsgIndex = "Wrong fullUsedNumIndex value",
				errMsgSize = "Wrong fullUsedNums size()";
			for (int j = 0; j < controls[currentNum][uopt].size(); j++){
				if (controls[currentNum][uopt][j] != -1 
					&& Workflows[0]->GetLevel(j,states[currentNum][j])==0){
					if (fullUsedNumIndex > fullUsedNums.size()-1) throw errMsg;
					if (fullUsedNums.size()==0) throw errMsgSize;
					stagesCores.push_back(make_tuple(j,i,fullUsedNums[fullUsedNumIndex]));
					fullUsedNumIndex++;
				}
			}
			currentNum = nextStateNumbers[currentNum][uopt];
		}
		f.close();
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}




// stages are numbered fom zero
void Model::GetStageInformation(int stage){
	int tbegin = stage * delta;
	string fname = "stage" + to_string((long long)stage+1) + ".txt";
	ofstream f(fname);
	int num = 0; // number of correct states for this stage
	// for all states
	int statesCount = GetStatesCount();
	for (int i = 0; i < statesCount; i++){
		f << "State " << i << endl;
		Workflows[0]->PrintState(states[i],f);
		int uopt = 0;
		double maxEff = 0.0; 
		vector<vector<pair<double, unsigned int>>> timeCoresPerType;
		timeCoresPerType.resize(Resources.size());
		vector<vector<int>> fullUsedNums;
		// if we have right core number for this state
		if (CheckState(i, stage, timeCoresPerType)) {
			vector<vector<int>>::const_iterator controlsIt = controls[i].begin();
			int controlIndex = 0;
			for (; controlsIt!=controls[i].end(); controlsIt++){
				timeCore currentTimeCore = timeCoresPerType;
				if (CheckControl(i, controlIndex, stage, currentTimeCore, fullUsedNums,false)){
					// if it is the last period
					double currEff = GetEfficiency(stage, currentTimeCore);
					if (stage == stages-1){
						if (currEff > maxEff){
							maxEff = currEff;
							uopt = controlIndex;
						}
					}
					else {
						double nextEff = FullInfo[stage+1][nextStateNumbers[i][controlIndex]].second;
						if (currEff + nextEff > maxEff){
							maxEff = currEff + nextEff;
							uopt = controlIndex;
						}
					}
				}
				controlIndex++;
			}
		}
		FullInfo[stage][i] = make_pair(uopt, maxEff);
		f << uopt << " ";
		Workflows[0]->PrintControl(controls[i][uopt], f);
		f << nextStateNumbers[i][uopt] << " ";
		Workflows[0]->PrintState(states[nextStateNumbers[i][uopt]], f);
		if (stage==0) break;
	}		
	
	//cout << "Stage " << stage << " has " << num << " states." << endl;
	f.close();

}

double Model::GetEfficiency(const int & stage, const timeCore& currentTC){
	double eff = 0.0;
	timeCore::const_iterator tcIt = currentTC.begin();
	for (;tcIt!=currentTC.end(); tcIt++){
		vector<pair<double,unsigned int>>::const_iterator pairsIt = tcIt->begin();
		for (;pairsIt!=tcIt->end(); pairsIt++){
			int tbegin = stage*delta;
			int tend = tbegin + (int)pairsIt->first;
			if (tend > T) tend = T;
			eff += EfficiencyByPeriod(pairsIt->second, tbegin, tend);
		}
	}
	return eff;
}

bool Model::CheckControl(const unsigned int &state, const unsigned int &control, const unsigned int &stage,
	timeCore& timeCoresPerType, vector <vector<int>>&fullUsedNums, bool isUsedNumsNeeded){
	try {
		string errMsg = "timeCoresPerType has wrong size";
		if (timeCoresPerType.size()!=Resources.size()) throw errMsg;
		Workflows[0]->SetTimesCoresForControl(states[state], controls[state][control], timeCoresPerType);
		vector<vector<pair<double, unsigned int>>>::const_iterator tCit = timeCoresPerType.begin();
		unsigned int typeIndex = 0;
		unsigned int inc = 0;
		for (;tCit != timeCoresPerType.end(); tCit++){
			vector <vector<int>> usedNums; 
			if (tCit->size()!=0){
				if (Resources[typeIndex]->Check(*tCit, stage, canExecuteOnDiffResources,
					usedNums, isUsedNumsNeeded)==false) 
					return false;
			}
			if (isUsedNumsNeeded){
				for (vector<vector<int>>::iterator packIt = usedNums.begin(); packIt!= usedNums.end(); packIt++){
					for (vector<int>::iterator coreIt = packIt->begin(); coreIt!=packIt->end(); coreIt++)
						*coreIt += inc;
				}
				inc += Resources[typeIndex]->GetCoresCount();
				copy(usedNums.begin(),usedNums.end(), back_inserter(fullUsedNums));
			}
			typeIndex++;	
		}
		return true;
	}
	catch (const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

bool Model::CheckState (const unsigned int state, const unsigned int stage, timeCore& timeCoresPerType){
	vector <vector<int>> fullUsedNums;
	Workflows[0]->SetTimesCoresForState(states[state], timeCoresPerType);
	vector<vector<pair<double, unsigned int>>>::const_iterator tCit = timeCoresPerType.begin();
	unsigned int typeIndex = 0;
	for (;tCit != timeCoresPerType.end(); tCit++){
		if (tCit->size()!=0){
			if (Resources[typeIndex]->Check(*tCit, stage, canExecuteOnDiffResources, fullUsedNums, false)==false) 
				return false;
		}
		typeIndex++;	
	}
	return true;
}




Model::~Model(void)
{
}
