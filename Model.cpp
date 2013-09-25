#include "StdAfx.h"
#include "Model.h"




bool mycompare (const OnePackageControl &lhs, const OnePackageControl &rhs){
	return get<0>(lhs) < get<0>(rhs);
}

bool packageStateOrder (const PackageState &lhs, const PackageState &rhs){
	return get<0>(lhs) < get<0>(rhs);
}

bool SortPairs(pair <int, float> p1, pair <int,float> p2){
	return (p1.second > p2.second);
}

void Model::Init (string resFile, string wfFile){
	InitResources(resFile); cout << "Initialization of resources ended" << endl;
	InitWorkflows(wfFile); 
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
			sprintf(second, "%d", line);
			errWrongFormatFull += second;
			throw errWrongFormatFull;
		}
		s.erase(0,trim.size());
		int workflowsCount = atoi(s.c_str());
		vector <int> types; map <pair <int,int>, float> execTime; vector <Package*> pacs; 
		vector <int> cCount; 
		vector <vector <int>> connectMatrix;
		int fullPackagesCount = 0;
		for (int i = 0; i < workflowsCount; i++){
			int packagesCount = 0;
			getline(file,s);
			++line;
			if (file.eof()) throw errEarlyEnd;
			
			if ((found = s.find("(")) == std::string::npos){
				sprintf(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			s.erase(0,found+1);
			istringstream iss(s);
			iss >> packagesCount;
			if (iss.fail()) {
				sprintf(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			if (packagesCount < 1) {
				sprintf(second, "%d", i+1);
				string beginStr = "Workflow ";
				beginStr += second;
				beginStr += " - ";
				beginStr += errPackagesCount;
				throw beginStr;
			}
			for (int j = 0; j < packagesCount; j++){
				float alpha = 0.0; // part of consequentually executed code
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
					sprintf(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				iss >> alpha;
				if (iss.fail()) {
					sprintf(second, "%d", line);
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
					sprintf(second, "%d", line);
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
						for (int i = 0; i < Resources.size(); i++)
							types.push_back(i+1);
						break;
					}
					if (iss.fail()) {
						sprintf(second, "%d", line);
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
					sprintf(second, "%d", line);
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
						sprintf(second, "%d", line);
						errWrongFormatFull += second;
						throw errWrongFormatFull;
					}
					cCount.push_back(coreCount);
					iss >> comma;
				} while (comma==",");
				
				if (cCount.size() < 1) {
					sprintf(second, "%d", i+1);
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
					sprintf(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				iss >> amount;
				if (iss.fail()) {
					sprintf(second, "%d", line);
					errWrongFormatFull += second;
					throw errWrongFormatFull;
				}

				for (int k = 0; k < types.size(); k++){
					for (int l = 0; l < cCount.size(); l++){
						// assume that the core numbers are in ascending order (else continue)
						if (Resources[k]->GetCoresCount() < cCount[l]) break; 
						// Amdal's law
						float acc = 1.00 / (alpha + (1-alpha)/(l+1));
						// execTime = amount / (perf * acc)
						float exTime = amount / (Resources[k]->GetPerf() * acc);
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
						sprintf(second, "%d", line);
						errWrongFormatFull += second;
						throw errWrongFormatFull;
					}
					if (val!=0  && val!=1){
						sprintf(second, "%d", i+1);
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
		for (int i = 0; i < Workflows.size(); i++){
			Workflows[i]->SetIsPackageInit();
			Workflows[i]->SetPackagesStates();
			int t = clock();
			Workflows[i]->SetFullPackagesStates(0);
			cout << "Time of SetFullPackagesStates() " << (clock()-t)/1000.0 << endl;
			t = clock();
			Workflows[i]->PrintPackagesStates();
			cout << "Time of PrintPackagesStates() " << (clock()-t)/1000.0 << endl;
			Workflows[i]->PrintControls();
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
			sprintf(second, "%d", line);
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
			sprintf(second, "%d", line);
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
			sprintf(second, "%d", i+1);
			string first = "Type ";
			trim = first + second;
			found = s.find(trim);
			if (found != 0) {
				sprintf(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			s.erase(0,trim.size()+2);
			iss.str(s);
			iss.clear();
			iss >> resourcesCount;
			if (iss.fail()) {
				sprintf(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			found = s.find(",");
			s.erase(0,found+2);
			iss.str(s);
			iss.clear();
			iss >> coresCount;
			if (iss.fail()) {
				sprintf(second, "%d", line);
				errWrongFormatFull += second;
				throw errWrongFormatFull;
			}
			float perf = 0.0;
			getline(file,s);
			trim = "Performance (GFlops): ";
			found = s.find(trim);
			if (found != 0) {
				sprintf(second, "%d", line);
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
					sprintf(second, "%d", k+1);
					first = "Core ";
					trim = first + second;
					found = s.find(trim);
					if (found != 0) {
						sprintf(second, "%d", line);
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
							sprintf(second, "%d", line);
							errWrongFormatFull += second;
							throw errWrongFormatFull;
						}
						iss >> two;
						if (iss.fail()) {
							sprintf(second, "%d", line);
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
		for (int i = 0; i < Resources.size(); i++) {
			Resources[i]->CorrectBusyIntervals(stageBorders);
			unsigned short fullCoresCount = Resources[i]->GetCoresCount();
			for (int j = 0; j < fullCoresCount; j++) typesCores.push_back(make_pair(i+1, j+1));
			Resources[i]->SetInitLastVals(initVal, initVal+fullCoresCount - 1);
			initVal +=fullCoresCount;
		}
		SetForcedBricks();
		int allCoresCount = 0;
		for (int i = 0; i < Resources.size(); i++) {
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
	for (int i = 0; i < Resources.size(); i++) {
		vector <int> resourceTypeForcedBricks;
		for (int j = 0; j < Resources[i]->GetResourceCount(); j++){
		currentVec = (*Resources[i])[j].GetForcedNumbers();
		//cout << "Current vec: ";
		for (int j = 0; j < currentVec->size(); j++) {
			//cout << (*currentVec)[j] << " ";
			(*currentVec)[j]+=beginVal;
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



//void Model::CheckForIllegalCoreNumber(vector<vector <OnePackageControl>> &fullPossibleControls){
//	vector <int> impossibleStateNumbers;
//	// eliminating impossible combinations
//	int number = 0;
//	for (vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin(); it1!= fullPossibleControls.end(); it1++){
//		
//			int isCoresNumberCorrect = true; 
//			vector <int> busyCores;
//			for (int i = 0; i < Nodes.size(); i++) busyCores.push_back(0);
//		 for (vector <OnePackageControl>::iterator it2 = (*it1).begin(); it2!= (*it1).end(); it2++){
//			 int node = (*it2).get<1>();
//			 int cores = (*it2).get<2>();
//			 if (node != 0) busyCores[node-1] += cores; 
//		 }
//		// file << "busy cores " << busyCores[0] << " " <<busyCores[1] << endl;
//		 for (int i = 0; i < Nodes.size(); i++)
//			 if (freeCores[i] < busyCores[i]) isCoresNumberCorrect = false;
//		 if (!isCoresNumberCorrect) {
//			impossibleStateNumbers.push_back(number);
//		 }
//		 number ++;
//	}
//
//	//cout << "Impossible states size :" << impossibleStateNumbers.size() << endl;
//
//	for (int i = 0; i < impossibleStateNumbers.size(); i++)
//	{
//		vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin();
//		int j = impossibleStateNumbers[i];
//		for (int k = i + 1; k < impossibleStateNumbers.size(); k++) --impossibleStateNumbers[k];
//		fullPossibleControls.erase(it1 + j);
//		//cout << impossibleStateNumbers[i] << " ";
//		//fullPossibleControls.erase(fullPossibleControls.begin() + 107);
//		
//	}
//}



//void Model::DirectBellman(){
//	cout << "Result: " << endl;
//	ofstream f("0.txt");
//	freeCores.clear();
//	SetFreeCores(0);
//	StageTable.clear();
//	cout << "Bellman scheme " << endl;
//	vector <PackageState> InitState;
//	for (int i = 1; i <=Packages.size(); i++)
//	for (int j = 0; j < Workflows.size(); j++){
//		if (Workflows[j]->isPackageIn(i)){
//			if(Workflows[j]->IsInit(i)) 
//				InitState.push_back(make_tuple(i,0,0,0));
//		else InitState.push_back(make_tuple(i,0,0,-1));
//		}
//	}
//	cout << "Stage 0 (state 0)" << endl;
//	f << "Stage 0" << endl;
//	for (int i = 0; i <Packages.size(); i++) cout << InitState[i] << " ";
//	// find number of init state in FullState, normally it is zero
//	int number = 0;
//	pControlNumbers.clear();
//	cout << endl;
//	PossibleControls pControls = CheckControls(number, 0); // vector of possible controls for this state
//	vector<int> uopts;
//	float efficiency;
//	// getting uopts
//	vector <OneControl>::iterator it = pControls.begin();
//	int nControl = 0; float maxEff = 0; 
//	float currentMaxEff = 0.0; int currentUopt = 0;
////	cout << "Finding uopts..." << endl;
//	StageInformation sti = FullInfo.back();
//	// for each possible control
//	for (;it!=pControls.end(); it++){
//		
//		f  << pControlNumbers[nControl] << " ";
//		float eff = GetEfficiency(*it, 0,number);
//		if (eff > currentMaxEff){
//			currentMaxEff = eff;
//			currentUopt = nControl;
//		}
//		f << eff << endl;
//		int nextStateNumber = FullNextState[number][pControlNumbers[nControl]];
//		float nextEff = 0.0; 
//
//		nextEff = sti[nextStateNumber].get<2>();
//		if (eff+nextEff == maxEff) uopts.push_back(pControlNumbers[nControl]);
//		if (eff + nextEff > maxEff){
//			uopts.clear();
//			maxEff = eff + nextEff;
//			uopts.push_back(pControlNumbers[nControl]);
//			efficiency = maxEff;
//		}
//		nControl ++;
//	}
//
//	StageTable.push_back(make_tuple(number,uopts,efficiency));
//	FullInfo.push_back(StageTable);
//	
//	int nextStateNumber = 0; 
//	
//	for (int i = 0; i < T/delta; i++){
//		
//		if (i) {
//			cout << "Stage " << i << " (state " << nextStateNumber << ")" << endl;
//			vector <PackageState> state = FullState[nextStateNumber];
//			for (int i = 0; i < state.size(); i++) cout << state[i] << " ";
//			cout << endl;
//			for (int j = 0; j < FullInfo[T/delta-i-1].size(); j++){
//				if (nextStateNumber == FullInfo[T/delta-i-1][j].get<0>())
//					uopts = FullInfo[T/delta-i-1][j].get<1>();
//			}
//		}
//		number = nextStateNumber;
//		cout << "Control for stage " << i << endl;
//		nextStateNumber = GetNextStateNumber(number, uopts[0], f);
//		OneControl control =  FullControl[number][uopts[0]] ;
//		
//		for (int i = 0; i < control.size(); i++) cout << control[i] << " ";
//		cout << endl;
//		cout << "Next state number = " << nextStateNumber << endl;
//		cout << endl;
//	}
//
//	
//	cout << endl;
//	GetMaxEfficiency();
//	cout << "Efficiency for Bellman scheme " << FullInfo[T/delta-1][0].get<2>()/maxEfficiency << endl;
//	///maxEfficiency << endl;
//	cout << endl;
//	cout << "Greedy algorithm " << endl;
//	float greedyEff = Greedy(pControlNumbers[currentUopt], currentMaxEff);
//	cout << "Efficiency for greedy algorithm  " << greedyEff/maxEfficiency << endl;
//	cout << endl << "Max efficiency " << maxEfficiency << endl << endl;
//	cout << "Advantage for Bellman: " << ((FullInfo[T/delta-1][0].get<2>())/maxEfficiency - greedyEff/maxEfficiency) * 100 << " %" << endl;
//	f.close();
//}


//void Model::GetStageInformation(int stage){
//	StageInformation result;
//	StageTable.clear();
//	int tbegin = stage * delta;
//	freeCores.clear();
//	SetFreeCores(tbegin);
//
//	char buf[10];
//	char * a = (char *)malloc(100*sizeof(char));
//	a = itoa(stage,buf,10);
//	char * c = ".txt";
//	char * name1 = strcat(a,c);
//	ofstream file(name1);
//	int num = 0; // number of correct states for this stage
//	// for all states
//	for (int i = 0; i < FullState.size(); i++){
//		pControlNumbers.clear();
//		vector <int> uopts;
//		float efficiency; 
//		// if we have right core number for this state
//		if (CheckState(i)) {
//			//double start = clock();
//			num ++;
//			PossibleControls pControls = CheckControls(i, tbegin);
//			// if it is the last period
//			if (tbegin == T - delta) uopts = GetUOptsByStateNumber(i, tbegin, file, efficiency); 
//			else{
//				 file << "State " << i << endl;
//				vector <OneControl>::iterator it = pControls.begin();
//				int nControl = 0; float maxEff = 0; int maxControl = 0,  numMax = 0;
//				// for each possible control
//				StageInformation &s = FullInfo.back();
//				for (;it!=pControls.end(); it++){
//					file << pControlNumbers[nControl] << " ";
//					float eff = GetEfficiency(*it, tbegin,i);
//					file << eff << endl;
//					//int nextStateNumber = GetNextStateNumber(i, nControl, file);
//					int nextStateNumber = FullNextState[i][pControlNumbers[nControl]];
//					float nextEff = 0.0; 
//					nextEff = s[nextStateNumber].get<2>();
//
//					/*for (int i = 0; i < s.size(); i++){
//						if (s[i].get<0>()==nextStateNumber)
//						{
//							nextEff = s[i].get<2>();
//							i = s.size();
//						}
//					}*/
//					if (eff+nextEff == maxEff) uopts.push_back(pControlNumbers[nControl]);
//					//cout << "Next eff = " << nextEff<< endl;
//					//cout << "Eff = " << eff<< endl;
//					if (eff + nextEff > maxEff){
//						uopts.clear();
//						maxEff = eff + nextEff;
//						uopts.push_back(pControlNumbers[nControl]);
//						efficiency = maxEff;
//					}
//					nControl ++;
//				}
//			}
//			StageTable.push_back(make_tuple(i,uopts,efficiency));
//			//cout << "Time of executing for one state" <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
//			
//		}
//		else StageTable.push_back(make_tuple(i,uopts,0.0));
//		
//	}
//	FullInfo.push_back(StageTable);
//	//cout << "before: " << fCount << endl;
//	//cout << "after: " << pCount << endl;
//	cout << "Stage " << stage << " has " << num << " states." << endl;
//	file.close();
//
//}

//PossibleControls Model::CheckControls(int stateNum, int tbegin){
//	PossibleControls pC = FullControl[stateNum];
//	vector <PackageState> s = FullState[stateNum];
//	// get free time ends for each node and each number of cores
//	map <int, vector <pair <int,int>>> freeTimeEnd, current; // (nodeNum, <(freeCoreNum1, endTime1), (freeCoreNum2, endTime2)...>)
//	
//	for (int i = 0; i < Nodes.size(); i++){
//		vector <pair <int,int>> freeTimesForNode;
//		int fullCoreCount = Nodes[i]->GetCoreCount();
//		for (int j = 0; j < fullCoreCount; j++)
//		{
//			freeTimesForNode.push_back(make_pair(j+1, Nodes[i]->GetFreeEnd(j+1,tbegin)));
//		}
//		freeTimeEnd.insert(make_pair(i+1,freeTimesForNode));
//	}
//
//	
//
//	PossibleControls result;
//	PossibleControls::iterator it = pC.begin();
//	int num = 0;
//	// for each controls
//	for (; it!= pC.end(); it++ ){
//		int flag = true;
//		vector <int> busyCores;
//		for (int i = 0; i < freeCores.size(); i++)
//			busyCores.push_back(0);
//		current.clear();
//		current = freeTimeEnd;
//		for (vector <OnePackageControl>::iterator ip = (*it).begin(); ip!= (*it).end(); ip++){
//			float level ;
//			int core = (*ip).get<2>();
//			int node = (*ip).get<1>();
//			if (node !=0) busyCores[node-1]+= core;
//			int package = (*ip).get<0>();
//
//			vector <PackageState>::iterator ps  = s.begin();
//			for (;ps!=s.end(); ps++)
//				if ((*ps).get<0>() == package) level = (*ps).get<3>();
//			if (level > 0 && node!=0) {
//				int fullCoreCount = Nodes[node-1]->GetCoreCount();
//				map<int,vector <pair <int,int>>>::iterator free = current.find(node);
//				int minusCount = 0;
//			
//				for (int j = fullCoreCount-1; j >=0; j--){
//					if (minusCount==core) break;
//					if (free->second[j].second!=0)
//					{
//						free->second[j].second=0;
//						++minusCount;
//							
//					}
//				}
//			}
//		}
//		// for each package in control
//		for (vector <OnePackageControl>::iterator ip = (*it).begin(); ip!= (*it).end(); ip++){
//			// check if we haven't enough cores;
//			
//			float level ;
//			int core = (*ip).get<2>();
//			int node = (*ip).get<1>();
//			if (node !=0) busyCores[node-1]+= core;
//			int package = (*ip).get<0>();
//			
//			vector <PackageState>::iterator ps  = s.begin();
//			for (;ps!=s.end(); ps++)
//				if ((*ps).get<0>() == package) level = (*ps).get<3>();
//
//			// if we prepare to begin execution 
//			if (level == 0 && node !=0){ //uncomment if will crash
//			
//				int execTime = Packages[package-1]->GetExecTime(node,core);
//				int tRealBegin = GetTRealBegin(execTime,level,tbegin);
//				map<int,vector <pair <int,int>>>::iterator free = current.find(node);
//				vector <pair <int,int>> freePair= free->second;
//				for (int i = 0; i < freePair.size(); i++){
//					if (core == freePair[i].first){
//						// if we don't have enough time
//						if (tRealBegin+execTime > freePair[i].second)
//						{ flag = false; break;}
//						// if we have enough time
//						else {
//							int fullCoreCount = Nodes[node-1]->GetCoreCount();
//							for (int j = fullCoreCount - core; j>=1; j-- ){
//								if (freePair[j-1].second > tRealBegin + execTime)
//									for (int k = j; k < fullCoreCount; k++){
//										//current[node-1][k-1].second = current[node-1][k].second;
//										free = current.find(node);
//										free->second[k-1].second = free->second[k].second;
//									}
//							}
//							for (int j = fullCoreCount - core + 1; j <= fullCoreCount; j++ ){
//								free = current.find(node);
//								free->second[j-1].second = 0;
//							}
//						}
//					}
//				}
//				
//			} // if
//		}//for
//		// uncomment if program will crash
//		/*for (int i = 0; i < freeCores.size(); i++){
//			if (busyCores[i]>freeCores[i]) flag = false;
//		}
//*/
//		if (flag) {
//			result.push_back(*it); 
//				//cout << num;
//			pControlNumbers.push_back(num);
//		}
//		num ++;
//	}// for
//	return result;
//}


//bool Model::CheckState (int stateNumber){
//	vector <int> busyCores;
//	for (int i = 0; i < freeCores.size(); i++) busyCores.push_back(0);
//	vector<PackageState>::iterator it = FullState[stateNumber].begin();
//	for (; it != FullState[stateNumber].end(); it++){
//		int nodeNum = (*it).get<1>();
//		if (nodeNum!=0) busyCores[nodeNum-1]+=(*it).get<2>();
//	}
//	for (int i = 0; i < freeCores.size(); i++)
//		if (freeCores[i] < busyCores[i]) return false;
//	return true;
//}
//
//
//
//float Model::GetEfficiency(OneControl &it2, int tbegin, int stateNum){
//	float e = 0;
//	// for each package in control
//	for (int i = 0; i < it2.size(); i++){
//		int nodeNum = it2[i].get<1>();
//		if (nodeNum!=0)
//		{
//			int packageNum = it2[i].get<0>();
//			float level = FullState[stateNum][packageNum-1].get<3>();
//			int coreNum = it2[i].get<2>();
//			int l = Packages[packageNum-1]->GetExecTime(nodeNum, coreNum);
//			int tend;
//			if (level == 0){
//				tend = tbegin + l;
//			}
//			else {
//				int tRealBegin = GetTRealBegin(l,level,tbegin);
//				tend = tRealBegin + l;
//			}
//			if (tend >= tbegin + delta) e+=EfficiencyByPeriod(coreNum, tbegin, tbegin + delta);
//			else e+=EfficiencyByPeriod(coreNum, tbegin, tend);
//			if (tend < tbegin){
//				cout << tbegin << endl;
//				cout << tend << endl;
//				cout << l << endl;
//				cout << level << endl;
//				cout << stateNum << endl;
//				for (int i = 0; i < it2.size(); i++) cout << it2[i] << " ";
//				cout << endl;
//			}
//		}
//	}
//	return e;
//}
//
//vector <int> Model::GetUOptsByStateNumber(int stateNum, int tbegin, ofstream &file, float &efficiency){
//	
//		file << "State " << stateNum << endl;
//		vector <int> result;
//		
//		// taking control variants for this state
//		vector <pair<int,float>> eff; // (number of control, efficiency value)
//		vector <OneControl> controlVector = FullControl[stateNum];
//		// нужно делать проверку
//
//		vector <OneControl>::iterator it2 = controlVector.begin();
//		int nControl = 0;
//		for (; it2!= controlVector.end(); it2++){
//			// if control is avaliable???
//			vector<int>::iterator v = find(pControlNumbers.begin(), pControlNumbers.end(), distance(controlVector.begin(), it2)) ; 
//			if ( v != pControlNumbers.end() )
//			{
//				
//				float e = GetEfficiency(*it2, tbegin, stateNum);
//				//int busyCores = GetBusyCores(*it2);
//				//file << busyCores << endl;
//				 eff.push_back(make_pair(nControl,e));
//				//file << eff.back();
//				//file << endl;
//			}	
//			nControl ++;
//		} 	
//			sort(eff.begin(), eff.end(), SortPairs);
//		
//			float maxVal = eff[0].second;
//			efficiency = maxVal;
//			int i = 0;
//			for (int i = 0; i < eff.size(); i++){
//				if (eff[i].second == maxVal) 
//					result.push_back(eff[i].first);
//				file << eff[i].first << " " << eff[i].second << endl;
//			}
//				
//		// if
//		
//	
//
//		/*while (eff[i].second == maxVal) {
//			result.push_back(eff[i].first);
//			file << eff[i].first << " " << eff[i].second << endl;
//			i++;
//			if (i == eff.size()-1) break;
//		}*/
//		//file.close();
//		return result;
//}
//
//int Model::GetTRealBegin(int l, float level, int periodBegin){
//	float tbegin = periodBegin - l*level;
//	for (int t = 0; t <=T; t+=delta){
//		if (tbegin >= t && tbegin < t+delta )
//			return t;
//	}
//}
//
//int Model::GetBusyCores(OneControl &control){
//	int cores = 0;
//	vector <OnePackageControl>::iterator it = control.begin();
//	for (;it!= control.end(); it++){
//		cores +=(*it).get<2>();
//	}
//	return cores;
//}



//int Model::GetNextStateNumber(int stateNum, int uOptNum, ofstream & file){
//	
//	
//	vector <PackageState> resultState;
//	
//	vector <PackageState> state = FullState[stateNum];
//	
//	for (int i = 0; i < state.size(); i++) file << state[i] << " ";
//	file << endl;
//	
//	OneControl one = FullControl[stateNum][uOptNum];
//	
//	sort(one.begin(),one.end(), mycompare);
//	
//	for (int i = 0; i < one.size(); i++) file << one[i] << " ";
//	file << endl;
//	
//	vector <int> unreadyPackagesNum;
//	vector <int> readyPackageNum;
//	// for each package
//	for (int i = 0; i < one.size(); i++){
//		
//		PackageState pS = state[i];
//		OnePackageControl pC = one[i];
//
//		int controlNode = pC.get<1>();
//		int controlCores = pC.get<2>();
//		int node = pS.get<1>();
//		int cores = pS.get<2>();
//		float level = pS.get<3>();
//
//		PackageState resState;
//
//		if ( node == 0  && controlNode == 0 && level == 0){
//			resultState.push_back(make_tuple(i+1,0,0,0)); 
//		}
//		else if ( controlNode != 0 ){
//			int t = (*(Packages[i])).GetExecTime(controlNode,controlCores);
//			float resLevel = level + (float)delta/(float)t;
//			if (resLevel > 1) resLevel = 1;
//			else {
//				int val = t/delta + 1;
//				float f = 1/(float)val;
//				for (float j = 0; j < 1; j+=f)
//				{
//					if (resLevel >= j && resLevel <= j+f){
//						if (abs(resLevel-j) < abs(j+f - resLevel))
//							resLevel = j;
//						else resLevel = j+f;
//					if (j + f == 1) resLevel = 1 - f;
//					}
//				}
//			}
//			if (resLevel == 1) readyPackageNum.push_back(i+1);
//			if (resLevel < 1) resultState.push_back(make_tuple(i+1,controlNode,controlCores,resLevel)); 
//			else resultState.push_back(make_tuple(i+1,0,0,resLevel)); 
//		}
//		else if (level == 1){
//			resultState.push_back(make_tuple(i+1,0,0,1));
//			readyPackageNum.push_back(i+1);
//		}
//		else
//			unreadyPackagesNum.push_back(i+1);
//		
//	}
//	
//	// for unready packages
//	vector <int>::iterator it = unreadyPackagesNum.begin();
//	for (;it!=unreadyPackagesNum.end(); it++){
//		bool isReady = true;
//		for (int i = 0; i < Workflows.size(); i++){
//			if ( Workflows[i]->isPackageIn(*it) ){
//				vector <int> depNumbers = Workflows[i]->GetDependency(*it);
//				for (int j = 0; j < depNumbers.size(); j++){
//					if (depNumbers[j]!=*it){
//					vector <int>::iterator ri = find(readyPackageNum.begin(),readyPackageNum.end(), depNumbers[j]);
//					if (ri == readyPackageNum.end() ) isReady = false;
//					}
//				}
//			}
//		}
//		if (isReady) resultState.push_back(make_tuple(*it,0,0,0));
//		else resultState.push_back(make_tuple(*it,0,0,-1));
//	}
//	sort(resultState.begin(),resultState.end(), packageStateOrder);
//	
//	
//	int sNum = GetStateNumber(resultState, stateNum); 
//	
//	
//	
//
//	file << sNum << " ";
//	for (int i = 0; i < resultState.size(); i++) file << resultState[i] << " ";
//	
//	file << endl;
//	file << endl;
//	
//	return sNum;
//}



//int Model::GetStateNumber(vector <PackageState> &resultState, int stateNum){
//	//ofstream file("error.txt",ios::app);
//	/*int count = 0;
//	for (vector <vector <PackageState>>::iterator it = FullState.begin()+stateNum + 1; it!= FullState.end(); it++)
//	{
//		vector <PackageState> findState = *it;
//		int flag = true;
//		for (int i = 0; i < findState.size(); i++){
//
//			if (resultState[i].get<0>()!=findState[i].get<0>()) flag = false;
//			else if (resultState[i].get<1>()!=findState[i].get<1>()) flag = false;
//			else if (resultState[i].get<2>()!=findState[i].get<2>()) flag = false;
//			if (resultState[i].get<3>()!=findState[i].get<3>()) flag = false;
//		}
//		if (flag) { return (distance(FullState.begin(),it));}
//		//count ++;
//	}
//	return -1;*/
//	vector <PackageState> particleState; int prevCount = 0; vector <int> stateNumbers;
//	// find the number of states
//	for (int i = 0; i < States.size(); i++){
//		for (int j = 0; j < Workflows[i]->Size(); j++){
//			particleState.push_back(resultState[j+prevCount]);
//			
//		}
//		prevCount+=particleState.size();
//		
//		for (int j = 0; j < States[i].size(); j++){
//			int flag = true;
//			for (int k = 0; k < particleState.size(); k++){
//				if (States[i][j][k]!=particleState[k]) {
//					flag = false; break;
//				}
//			}
//			if (flag == true) {
//				stateNumbers.push_back(j); //(j)
//				break;
//			}
//		}
//		particleState.clear();
//	}
//	int resultNum = 0;
//	for (int i = 0; i < stateNumbers.size(); i++){
//		resultNum+=stateNumbers[i]*repeatCounts[i];
//	}
//	//cout << resultNum << endl;
//	// find the number of Full State
//	/*for (int i = 0; i < stateNumbers.size(); i++){
//		int sNum = stateNumbers[i];
//		resultNum+=sNum*repeatCounts[i];
//		int diff = 0;
//		for (std::multimap<int,pair<int,int>>::iterator it=RepeatChanges[i].begin(); it!=RepeatChanges[i].end(); ++it){
//			if ((*it).first <=resultNum) {
//				resultNum-=  (*it).second.second;
//			}
//			else break;
//		}
//		
//	}*/
//	if (diff.size()!=0){
//	for (int i = 0; i < diff.size()-1; i++){
//		if (diff[i].first <= resultNum && diff[i+1].first > resultNum){
//			resultNum -= diff[i].second;
//			break;
//		}
//	}
//	
//		if (diff.back().first <= resultNum) 
//		resultNum -=diff.back().second;
//	}
//	//cout << resultNum << endl;
//	return resultNum;
//}



//float Model::Greedy(int currentUopt, float currentEff){
//	ofstream f("zhadny.txt");float maxEff = 0.0;
//	
//	int stateNum = 0;
//	int nextState = 0;
//	int tbegin = 0;
//	for (int i = T/delta-1; i >=0 ; i--){
//		int uopt = 0;
//		StageInformation &si = FullInfo[i];
//		if (i == T/delta-1){
//			uopt = currentUopt;
//			maxEff+=currentEff;
//			nextState = FullNextState[stateNum][uopt];
//			
//			
//		}
//		else{
//			pControlNumbers.clear();
//			float uopteff = 0.0;
//			PossibleControls pc = CheckControls(nextState, tbegin);
//			for (int j = 0; j <pc.size(); j++){
//				
//				float eff = GetEfficiency(pc[j], tbegin,nextState);
//				if (eff > uopteff){
//					uopt = pControlNumbers[j];
//					uopteff = eff;
//				}
//			}
//			
//			maxEff += uopteff;
//			stateNum = nextState;
//			nextState = FullNextState[nextState][uopt];
//			
//		}
//
//		vector <PackageState> p = FullState[stateNum];
//		for (int j = 0; j < Packages.size();j++){
//			cout << p[j] << " ";
//		}
//		cout << endl;
//		OneControl c = FullControl[stateNum][uopt];
//			for (int j = 0; j < Packages.size();j++){
//			cout << c[j] << " ";
//		}
//		cout << endl;
//		cout << "Next state num: " << nextState << endl;
//		tbegin +=delta;
//	}
//	
//	return maxEff;
//}
//
//


Model::~Model(void)
{
}
