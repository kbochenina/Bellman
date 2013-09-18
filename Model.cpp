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
	InitWorkflows(wfFile); cout << "Initialization of workflows ended" << endl;
}

void Model::InitWorkflows(string f){}


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
		
		for (int i = 0; i < Resources.size(); i++) 
			Resources[i]->CorrectBusyIntervals(stageBorders);
		SetForcedBricks();
		for (int i = 0; i < Resources.size(); i++) {
			Resources[i]->SetFreeTimeEnds();
			Resources[i]->SetChildForcedBricks();
		}
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

//void Model::Init(){
//	// first node
//	//int coreCount = 4;
//	int coreCount = 2;
//	int priority = 1;
//	multimap <int,pair<int,int>> busyIntervals;
//	busyIntervals.insert(make_pair(1, make_pair(16401,23653))); 
//	busyIntervals.insert(make_pair(2, make_pair(23653,24817))); 
//	busyIntervals.insert(make_pair(1, make_pair(24817,27603)));
//	busyIntervals.insert(make_pair(1, make_pair(30973,31051)));
//	//busyIntervals.insert(make_pair(2, make_pair(0,7200))); 
//	Nodes.push_back(new Node(coreCount, priority, busyIntervals)); busyIntervals.clear();
//	/*busyIntervals.insert(make_pair(1, make_pair(29024,  32459))); 
//	busyIntervals.insert(make_pair(1, make_pair(39289,  43200))); */
//	//Nodes.push_back(new Node(coreCount, priority, busyIntervals));busyIntervals.clear();
//	/*busyIntervals.insert(make_pair(1, make_pair(23610,26207))); 
//	Nodes.push_back(new Node(coreCount, priority, busyIntervals));busyIntervals.clear();*/
//
//	int arr[15]= {5104, 2055, 6178, 3633, 6068, 10456, 6753, 1097, 1780, 4764, 4984, 3467, 10336, 10722, 1596};
//	int index = 0;
//	for (int i = 1; i <=15; i++) {
//		int uid = i;
//		vector <int> nodeNumbers;
//		vector <int> coreCounts;
//		coreCounts.push_back(1);
//		map <pair<int,int>, int> execTime;
//		for (int j = 0; j < 1; j++) {
//			nodeNumbers.push_back(j+1);
//			execTime.insert(make_pair(make_pair(j+1,1), arr[index]));
//		}
//		index++;
//		Packages.push_back(new Package(uid, nodeNumbers, coreCounts, execTime));
//	}
//	
//	// workflow 1
//	vector <int> packageNumbers; 
//	map <int, vector<int>> links, directLinks;
//		
//	for (int i = 0; i <3; i++){
//		for (int j = 1; j <=5; j++){
//			packageNumbers.push_back(i*5+j);
//		}
//		vector <int> dp;
//		dp.push_back(4+i*5);dp.push_back(5+i*5);links.insert(make_pair(1+i*5, dp)); dp.clear(); 
//		dp.push_back(4+i*5); dp.push_back(5+i*5);links.insert(make_pair(2+i*5, dp)); dp.clear(); 
//		dp.push_back(4+i*5);dp.push_back(5+i*5); links.insert(make_pair(3+i*5, dp)); dp.clear(); 
//		dp.push_back(5+i*5); links.insert(make_pair(4+i*5, dp)); dp.clear(); 
//		
//		vector<int> dLinks;
//		dLinks.push_back(4+i*5); directLinks.insert(make_pair(1+i*5, dLinks)); dLinks.clear();
//		dLinks.push_back(4+i*5); directLinks.insert(make_pair(2+i*5, dLinks)); dLinks.clear(); 
//		dLinks.push_back(4+i*5); directLinks.insert(make_pair(3+i*5, dLinks)); dLinks.clear(); 
//		dLinks.push_back(5+i*5); directLinks.insert(make_pair(4+i*5, dLinks)); dLinks.clear(); 
//		
//		Workflows.push_back(new Workflow(packageNumbers,links,directLinks));
//		packageNumbers.clear();
//		links.clear();
//		directLinks.clear();
//	}
//		
//	directLinks.clear();links.clear(); packageNumbers.clear();
//	
//	allCoresCount = 0;
//	for (int i = 0; i < Nodes.size(); i++){
//		allCoresCount+=Nodes[i]->GetCoreCount();
//	}
//	koeff = 2.00/allCoresCount;
//	
//}

void Model::SetFreeCores(int tbegin){
	for (int i = 0; i < Nodes.size(); i++)
	{
		
		freeCores.push_back((*(Nodes[i])).GetFreeCores(tbegin));
	}
}


//void Model::SetFullState(){
//	// and another foolish comment
//	cout << "Generating states and controls..." << endl;
//	ofstream file("fullState.txt");
//	ofstream fileControl("fullControl.txt");
//	ofstream fileNext("fullNext.txt");
//	
//	int num = 0, realNum = 0, workflowsCount = 0, k = 10, allStateCount = 1, ucount = 0;
//
//	workflowsCount = Workflows.size();
//	for (int i = 0; i < Nodes.size(); i++)
//		freeCores.push_back(Nodes[i]->GetCoreCount());
//
//	State oneState;
//	// creating states for each workflow
//	for (int i = 0; i < workflowsCount; i++){
//		int numberFirst = 0;
//		char buf[10];
//		char name[100];
//		strcpy(name,"Workflow");
//		strcat(name,itoa(i+1,buf,10));
//		strcat(name,".txt");
//		vector <int> packageNumbers = Workflows[i]->GetPackageNumbers();
//		oneState = GetStates(name, packageNumbers, freeCores, packageNumbers.size());
//		States.push_back(oneState);
//		
//		allStateCount *= oneState.size();
//	}
//	
//	
//	cout << allStateCount << endl;
//
//	vector <int> currentCounts;
//	repeatCounts.push_back(1); currentCounts.push_back(0);
//	
//	for (int i = workflowsCount - 2; i >= 0; i--){
//		repeatCounts.push_back(repeatCounts.back() * States[i+1].size());
//		currentCounts.push_back(0);
//	}
//	reverse(repeatCounts.begin(), repeatCounts.end());
//	vector <vector <int>> stateNumbers;
//	vector <PackageState> fullState;
//	
//	int missing = 0; int prevCount = 0; 
//
//	while (num < allStateCount){
//		//cout << num << endl;
//		vector <PackageState>::iterator ip;
//		
//		for (int i = 0; i < repeatCounts.size(); i++){
//			if (num >= repeatCounts[i] && (num % repeatCounts[i]==0)) {
//				currentCounts[i]++; 
//				if (currentCounts[i] > States[i].size() - 1) {
//					currentCounts[i]=0;
//					// beginning of repeating cycle
//					//beginRepeat[i]=realNum;
//				}
//			}
//		}
//		for (int i = 0; i < workflowsCount; i++)
//		{
//			int stateNum = currentCounts[i];
//			for (ip = States[i][stateNum].begin(); ip!=States[i][stateNum].end(); ip++)
//				fullState.push_back(*ip);
//		}
//		
//		
//		int flag = true;
//		for (int i = 0; i < Nodes.size(); i++)
//		{
//			// max value of cores
//			int max = freeCores[i];
//			int oneCoreCount = 0;
//			vector <PackageState>::iterator it3 = fullState.begin();
//			// for all packages states
//			for (it3; it3!=fullState.end(); it3++){
//				int oneNode = (*it3).get<1>();
//				if (oneNode == i+1) oneCoreCount +=(*it3).get<2>();
//				if (oneCoreCount > max) {
//					flag = false;
//					missing++;
//					break;
//				}
//			}
//			if (!flag) {
//				//if (missing==0) 
//				//prevCount = missing;
//			break;
//			}
//		}
//		if (flag == true) {
//			if (missing>0) {
//				if (diff.size()>0) diff.push_back(make_pair(num,diff.back().second+missing));
//				else diff.push_back(make_pair(num,missing));
//			}
//			missing = 0;
//			file << realNum << " ";
//			for (vector<PackageState>::iterator i = fullState.begin(); i!=fullState.end(); i++)
//				file << (*i) << " " ;
//			file << endl;
//			fileControl << "State " << realNum << endl;
//			FullState.push_back(fullState); 
//			PossibleControls controls = GetStateControls(realNum, fileControl);
//			//cout << controls.size() << endl;
//			ucount += controls.size();
//			FullControl.push_back(controls);
//			/*for (int i = 0; i < workflowsCount; i++){
//				int diff = currentCounts[i]-prevCount[i];
//				if (diff > 1){
//					RepeatChanges[i].insert(make_pair(beginRepeat[i],make_pair(currentCounts[i]-1, repeatCounts[i])));
//				}
//			}*/
//			//prevCount = currentCounts;
//			realNum++;
//		}
//		int percent = (float)num / (float)allStateCount * 100;
//		if (percent >= k) { cout << k << " % " << "...\n"; k+=10;}
//		
//		fullState.clear();
//		num ++;
//	}
//	
//
//	cout << 100 << " % " << "...\n";
//	k = 10;
//	cout << "All possible states count is: " << FullState.size() << "." << endl;
//	cout << "All possible controls count is: " << ucount << "." << endl;
//	ofstream f("error.txt");
//	for (int i = 0; i < diff.size(); i++)
//		f << diff[i].first << " " << diff[i].second << endl;
//	float max_time = 0; int prevNum = 0;
//	//for (int i = 0; i < FullState.size(); i++){
//	//	vector <PackageState> state = FullState[i];
//	//	//for (int j = 0; j < state.size(); j++) cout << i << " " << state[j] << " ";
//	//	//cout << endl;
//	//	int s = GetStateNumber(state, i);
//	//	//f << s << endl;
//	//	//if (i!=s) {
//	//		cout<< s << " "<< i <<  endl;
//	//	//}
//	//	
//	//	//cout << "Time for getting state number " <<  i << " " << time  << " sec "<< endl;
//	//	//if (time > max_time) {max_time = time;  cout << i << " Max_time = " << max_time << endl;}
//	//}
//
//	f.close();
//	cout << "Generating next states for all combinations of states and controls...\n";
//	int fSize = FullState.size(); 
//	for (int i = 0; i < FullState.size(); i++){
//		fileNext << "State " << i << endl;
//		vector <int> v; PossibleControls controls = FullControl[i];
//		for (int j = 0 ; j < controls.size(); j++){
//			//cout << "Control " << j+1	<< " of " << controls.size()-1 << endl;
//			//double start = clock();
//			v.push_back(GetNextStateNumber(i, j, fileNext));
//			//float time = (clock() - start) / CLOCKS_PER_SEC;
//			//cout << "Time of executing " <<  time  << " sec "<< endl;
//			//if (time > max_time) {max_time = time;  cout << "Max_time = " << max_time << endl;}
//		}
//		FullNextState.insert(make_pair(i, v));
//		int percent = (float)i / (float)fSize * 100;
//		if (percent >= k) { cout << k << " % " << "...\n"; k+=10;}
//		
//	}
//	cout << 100 << " % " << "...\n";
//	file.close();
//	fileNext.close();
//	fileControl.close();
//}

//State Model::GetStates(const char *filename, vector <int> packages, vector <int> freeCores, int workflowSize){
//	State result; State oneResult,  otherResults;
//	ofstream file(filename);
//	vector <PackageState> oneState, fullState;
//	int package, node, core; float level;
//	// if we have one package
//	if (packages.size() == 1) {
//		package = packages.back();
//		node = 0;
//		core = 0;
//		level = -1;
//		int flagNotInit = true;
//		for (int i = 0; i < Workflows.size(); i++){
//			if (Workflows[i]->isPackageIn(package) && Workflows[i]->IsInit(package)) 
//				flagNotInit = false; }
//		if (flagNotInit) {
//		oneState.push_back(make_tuple(package, node, core, level));
//		result.push_back(oneState);
//		//for (vector<PackageState>::iterator i = oneState.begin(); i!=oneState.end(); i++)
//			//file  << (*i) << " " ; file << endl;
//		oneState.clear();
//		}
//		node = 0;
//		core = 0;
//		level = 0;
//		oneState.push_back(make_tuple(package, node, core, level));
//		result.push_back(oneState);
//		//for (vector<PackageState>::iterator i = oneState.begin(); i!=oneState.end(); i++)
//		//	file  << (*i) << " " ; file << endl;
//		
//		//for (vector<PackageState>::iterator i = oneState.begin(); i!=oneState.end(); i++)
//		//	file  << (*i) << " " ; file << endl;
//		oneState.clear();
//		
//		vector <int> possibleNodes = Packages[package-1]->GetNodeNumbers();
//		vector <int> possibleCores = Packages[package-1]->GetCoreCounts();
//		for (int i = 0; i < possibleNodes.size(); i++)
//		{
//			int nodeNumber = possibleNodes[i]; // number of node
//			for (int j = 0; j < possibleCores.size(); j++)
//			{
//				int l = Packages[package-1]->GetExecTime(nodeNumber,possibleCores[j]);
//				if (freeCores[nodeNumber-1] >= possibleCores[j])
//				{
//					int val = l/delta + 1;
//					float f = 1/(float)val;
//					for (float k = f; k < 1; k+= f)
//					{
//						if (l >=delta){
//						oneState.push_back(make_tuple(package,nodeNumber, j+1, k));
//						result.push_back(oneState);
//						//for (vector<PackageState>::iterator i = oneState.begin(); i!=oneState.end(); i++)
//						//file << (*i) << " " ;
//						//file << endl;
//						oneState.clear();
//						}
//					}
//				}
//			}
//		}
//		oneState.clear();
//		node = 0;
//		core = 0;
//		level = 1;
//		oneState.push_back(make_tuple(package, node, core, level));
//		result.push_back(oneState);
//		
//		//result.push_back(oneState);
//	}
//	// if we have more than one package 
//	else {
//		bool flag = true;
//		vector <int> onePackage, otherPackages;
//		onePackage.push_back(packages[0]);
//		for (int i = 1; i < packages.size(); i++) otherPackages.push_back(packages[i]);
//		oneResult = GetStates(filename, onePackage, freeCores,workflowSize);
//		otherResults = GetStates(filename, otherPackages, freeCores, workflowSize);
//		
//		vector <vector <PackageState>>::iterator it1, it2;
//		//cout << "size = " << packages.size() << endl;
//		for (it1 = oneResult.begin(); it1!=oneResult.end(); it1++)
//		{
//			for (it2 = otherResults.begin(); it2!=otherResults.end(); it2++)
//			{
//				bool flag = true;
//				//vector <bool> flags;
//				//for (int i = 0; i < otherPackages.size(); i++) flags.push_back(true);
//				// check for model constraints
//				int onePackageNum = (*it1)[0].get<0>();
//				vector <PackageState>::iterator it3 = (*it2).begin();
//				// for all other packages states
//				for (it3; it3!=(*it2).end(); it3++){
//					if (flag!=false)
//					{
//						int otherPackageNum = (*it3).get<0>(); 
//						// for all workflows
//						for (int i = 0; i < Workflows.size(); i++){
//							// if packages are in the same workflow
//							if (Workflows[i]->isPackagesBothIn(onePackageNum,otherPackageNum))
//							{
//								// and otherPackage depends on onePackage
//								if (Workflows[i]->isDepends(onePackageNum,otherPackageNum))
//								{
//									//cout << "First: " << (*it1)[0] << " Second: " << *it3 << endl;
//									if (Workflows[i]->IsInit(onePackageNum) && (*it1)[0].get<3>()==-1) flag =  false;
//									if ((*it1)[0].get<3>()<1 && (*it3).get<3>()!=-1) flag = false;
//									if ((*it1)[0].get<3>()!=1 && (*it3).get<3>()!=-1) flag = false;
//									if ((*it1)[0].get<3>()==-1 && (*it3).get<3>()!=-1) flag = false;
//									if ((*it1)[0].get<3>()==1 &&(*it3).get<3>()==-1) {
//										if (otherPackages.size()+1 == workflowSize){
//											flag = false;
//											vector <PackageState>::iterator addIt = (*it2).begin();
//											int numIsReady = 0, numDepends = 0;
//											for (; addIt !=(*it2).end() ; addIt++)
//											{
//												int addPackageNum = (*addIt).get<0>(); 
//												int basePackageNum = (*it3).get<0>();
//												if (addPackageNum!=basePackageNum){
//													if (Workflows[i]->isDepends(addPackageNum,basePackageNum)){
//														numDepends ++;
//														if ((*addIt).get<3>()!=1) flag = true;
//														else if ((*addIt).get<3>()==1) numIsReady++;
//													}
//												}
//											}
//											if (numIsReady == numDepends) flag = false;
//										}
//									}
//									if ((*it1)[0].get<3>()==-1 && (*it3).get<3>()==-1) flag = true;
//									
//									//file << (*it1)[0] << " and " << *it3 << " = " << flag << endl;
//								}
//							}
//						} // for
//					} // if
//				} // for
//				// check for cores constraints
//				// for all nodes
//				if (flag == true) {
//					for (int i = 0; i < Nodes.size(); i++)
//					{
//						// max value of cores
//						int max = Nodes[i]->GetCoreCount();
//						int oneNode = (*it1)[0].get<1>(), oneCoreCount = 0;
//						if (oneNode == i+1) oneCoreCount = (*it1)[0].get<2>();
//						vector <PackageState>::iterator it3 = (*it2).begin();
//						// for all other packages states
//						for (it3; it3!=(*it2).end(); it3++){
//							oneNode = (*it3).get<1>();
//							if (oneNode == i+1) oneCoreCount +=(*it3).get<2>();
//							if (oneCoreCount > max) { flag = false; }
//							//if (flag==true) cout << (*it1)[0] << " and " << *it3 << " = " << flag << endl;
//						}
//					}
//				}
//				if (flag == true )// && oneState.size()!=0) 
//				{
//					fullState.push_back((*it1)[0]);
//					for (it3 = (*it2).begin(); it3!=(*it2).end(); it3++){
//						fullState.push_back(*it3);
//					}
//					
//					/*if (fullState.size() == packages.size())
//					{
//										
//						cout << endl;
//					}*/
//					result.push_back(fullState);
//					fullState.clear();
//				}
//				
//			}
//		}
//	}
//	vector <vector <PackageState>>::iterator it = result.begin();
//	for (; it!=result.end(); it++){
//		for (vector <PackageState>::iterator j = it->begin(); j!= it->end(); j++)
//			file << *j << " ";
//		file << endl;
//	}
//	file.close();
//	
//	return result;
//}

void Model::CheckForIllegalCoreNumber(vector<vector <OnePackageControl>> &fullPossibleControls){
	vector <int> impossibleStateNumbers;
	// eliminating impossible combinations
	int number = 0;
	for (vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin(); it1!= fullPossibleControls.end(); it1++){
		
			int isCoresNumberCorrect = true; 
			vector <int> busyCores;
			for (int i = 0; i < Nodes.size(); i++) busyCores.push_back(0);
		 for (vector <OnePackageControl>::iterator it2 = (*it1).begin(); it2!= (*it1).end(); it2++){
			 int node = (*it2).get<1>();
			 int cores = (*it2).get<2>();
			 if (node != 0) busyCores[node-1] += cores; 
		 }
		// file << "busy cores " << busyCores[0] << " " <<busyCores[1] << endl;
		 for (int i = 0; i < Nodes.size(); i++)
			 if (freeCores[i] < busyCores[i]) isCoresNumberCorrect = false;
		 if (!isCoresNumberCorrect) {
			impossibleStateNumbers.push_back(number);
		 }
		 number ++;
	}

	//cout << "Impossible states size :" << impossibleStateNumbers.size() << endl;

	for (int i = 0; i < impossibleStateNumbers.size(); i++)
	{
		vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin();
		int j = impossibleStateNumbers[i];
		for (int k = i + 1; k < impossibleStateNumbers.size(); k++) --impossibleStateNumbers[k];
		fullPossibleControls.erase(it1 + j);
		//cout << impossibleStateNumbers[i] << " ";
		//fullPossibleControls.erase(fullPossibleControls.begin() + 107);
		
	}
}

//PossibleControls Model::GetStateControls(int stateNumber, ofstream& file){
//	vector <PackageState> &state = FullState[stateNumber];
//	for (vector <PackageState>::iterator i5 = state.begin(); i5!= state.end(); i5++)
//			file << (*i5) << " ";
//			file << endl;
//	PossibleControls fullPossibleControls;
//	vector <OnePackageControl> OneControl;
//	vector <PackageState>::iterator it = state.begin();
//	vector <int> fc;
//	for (int i = 0; i < freeCores.size(); i++) fc.push_back(freeCores[i]);
//	// for each package in state
//	for (; it!=state.end(); it++){
//		PackageState pState = (*it);
//		int pNum = (*it).get<0>();
//		int node = (*it).get<1>();
//		int cores = (*it).get<2>();
//		int level = (*it).get<3>();
//		// if package is executing
//		if (node > 0) {
//			OneControl.push_back(make_tuple(pNum, node,cores));
//			//file << make_tuple(pNum, node,cores) << endl;
//			fc[node-1]-=cores;
//			if (fc[node-1] < 0) fc[node-1]=0;
//			//file << "fc = "<< fc[node-1] << " endl";
//		}
//		// if package cannot execute
//		if (level == -1 || level == 1){
//			OneControl.push_back(make_tuple(pNum, 0,0));
//			//file << make_tuple(pNum, 0,0) << endl;
//		}
//	
//	}
//	//file << "Possible packages controls " << endl;
//	vector <vector <OnePackageControl>> possiblePackagesControl;
//	// if package can execute
//	int possibleControlsCount = 1;
//	for (it = state.begin(); it!=state.end(); it++){
//		/*for (vector <int>::iterator i = fc.begin(); i!= fc.end(); i++)
//			file << "<" << *i << ">";
//		file << endl;*/
//		vector <OnePackageControl> onePackageControls;
//		int pNum = (*it).get<0>();
//		int node = (*it).get<1>();
//		int cores = (*it).get<2>();
//		float level = (*it).get<3>();
//		if (level == 0)
//		{
//			vector <int> possibleNodes = Packages[pNum-1]->GetNodeNumbers();
//			vector <int> possibleCores = Packages[pNum-1]->GetCoreCounts();
//			for (int i = 0; i < possibleNodes.size(); i++)
//			{
//				int nodeNumber = possibleNodes[i]; // number of node
//				for (int j = 0; j < possibleCores.size(); j++)
//				{
//					if (fc[nodeNumber-1] >= possibleCores[j])
//					{
//						onePackageControls.push_back(make_tuple(pNum, nodeNumber, possibleCores[j]));
//						//file << make_tuple(pNum, nodeNumber, possibleCores[j]);
//			
//					} // if
//				} // for
//			} // for
//			onePackageControls.push_back(make_tuple(pNum, 0, 0));
//			/*cout << pNum << " : ";
//			for (vector <OnePackageControl>::iterator it = onePackageControls.begin(); it!= onePackageControls.end(); it++)
//				cout << *it << " ";
//			cout << endl;*/
//			possiblePackagesControl.push_back(onePackageControls);
//			possibleControlsCount *= onePackageControls.size();
//			//if (onePackageControls.size() == 1) possibleControlsCount++;
//		}
//	}
//
//	//vector<vector <OnePackageControl>> fullPossibleControls;
//	// generating possible combinations of controlling
//		
//	vector <vector <OnePackageControl>>::iterator iP = possiblePackagesControl.begin();
//	int repeatCount = 1, n = 1;
//	for (;iP!=possiblePackagesControl.end();iP++){
//		
//		// for all possible controls
//		int packageStateNumber = 0;
//		for (int i = 0; i < possibleControlsCount; i++){
//			// if it is the first package, we must create new vector 
//			if (repeatCount == 1 && iP == possiblePackagesControl.begin() ){
//				vector <OnePackageControl> newControl;
//				newControl.push_back((*iP)[packageStateNumber++]);
//				fullPossibleControls.push_back(newControl);
//			}
//			else{
//				for (int j = 1; j <= repeatCount; j++)
//				{
//					fullPossibleControls[i++].push_back((*iP)[packageStateNumber]);
//					if (packageStateNumber == (*iP).size()) packageStateNumber = 0;
//				}
//				packageStateNumber++;
//				--i;
//			}
//			if (packageStateNumber == (*iP).size()) packageStateNumber = 0;	
//			
//		}
//		repeatCount*=(*iP).size();
//	}
//	/*for (vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin(); it1!= fullPossibleControls.end(); it1++){
//		 file << n << " : ";
//		 for (vector <OnePackageControl>::iterator it2 = (*it1).begin(); it2!= (*it1).end(); it2++)
//			file  << *it2 << " ";
//			file << endl;
//	}*/
//	//cout << fullPossibleControls.size() << endl;
//	CheckForIllegalCoreNumber(fullPossibleControls);
//	//cout << fullPossibleControls.size() << endl;
//
//
//	//file << "After eliminating: " << endl;
//	//for (vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin(); it1!= fullPossibleControls.end(); it1++){
//	//		vector <int> busyCores; for (int i = 0; i < Nodes.size();i++) busyCores.push_back(0); 
//	//	 for (vector <OnePackageControl>::iterator it2 = (*it1).begin(); it2!= (*it1).end(); it2++){
//	//		 int node = (*it2).get<1>();
//	//		 int cores = (*it2).get<2>();
//	//		 if (node != 0) busyCores[node-1] += cores; 
//	//		 //file << *it2 << " ";
//	//	 }
//	//		//file << "busy cores " << busyCores[0] << " " <<busyCores[1] << endl;
//	//		//file << endl;
//	//}
//	// gathering full pack of control for this state
//		if (fullPossibleControls.size() == 0) fullPossibleControls.push_back(OneControl);
//		else {
//		for (vector <OnePackageControl>::iterator it = OneControl.begin(); it!= OneControl.end(); it++)
//		for (int i = 0; i < fullPossibleControls.size(); i++){ 
//				fullPossibleControls[i].push_back(*it);
//			//cout << i << endl;
//			}
//		}
//	
//	// checking for illegal number of cores
//	CheckForIllegalCoreNumber(fullPossibleControls);
//	
//	//file << "After gathering: " << endl;
//	for (vector<vector <OnePackageControl>>::iterator it1 = fullPossibleControls.begin(); it1!= fullPossibleControls.end(); it1++){
//		 //vector <int> busyCores; for (int i = 0; i < Nodes.size();i++) busyCores.push_back(0); 
//		 sort((*it1).begin(),(*it1).end(), mycompare);
//		 for (vector <OnePackageControl>::iterator it2 = (*it1).begin(); it2!= (*it1).end(); it2++){
//			
//			int node  = (*it2).get<1>();
//			int cores = (*it2).get<2>();
//			//if (node!=0) busyCores[node-1] += cores;
//			
//			file << *it2 << " ";
//			
//		 }
//			//file << " busy cores " << busyCores[0] << " " <<busyCores[1] << endl;
//		file << endl;
//	}
//	
//	return fullPossibleControls;
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


void Model::GetStageInformation(int stage){
	StageInformation result;
	StageTable.clear();
	int tbegin = stage * delta;
	freeCores.clear();
	SetFreeCores(tbegin);

	char buf[10];
	char * a = (char *)malloc(100*sizeof(char));
	a = itoa(stage,buf,10);
	char * c = ".txt";
	char * name1 = strcat(a,c);
	ofstream file(name1);
	int num = 0; // number of correct states for this stage
	// for all states
	for (int i = 0; i < FullState.size(); i++){
		pControlNumbers.clear();
		vector <int> uopts;
		float efficiency; 
		// if we have right core number for this state
		if (CheckState(i)) {
			//double start = clock();
			num ++;
			PossibleControls pControls = CheckControls(i, tbegin);
			// if it is the last period
			if (tbegin == T - delta) uopts = GetUOptsByStateNumber(i, tbegin, file, efficiency); 
			else{
				 file << "State " << i << endl;
				vector <OneControl>::iterator it = pControls.begin();
				int nControl = 0; float maxEff = 0; int maxControl = 0,  numMax = 0;
				// for each possible control
				StageInformation &s = FullInfo.back();
				for (;it!=pControls.end(); it++){
					file << pControlNumbers[nControl] << " ";
					float eff = GetEfficiency(*it, tbegin,i);
					file << eff << endl;
					//int nextStateNumber = GetNextStateNumber(i, nControl, file);
					int nextStateNumber = FullNextState[i][pControlNumbers[nControl]];
					float nextEff = 0.0; 
					nextEff = s[nextStateNumber].get<2>();

					/*for (int i = 0; i < s.size(); i++){
						if (s[i].get<0>()==nextStateNumber)
						{
							nextEff = s[i].get<2>();
							i = s.size();
						}
					}*/
					if (eff+nextEff == maxEff) uopts.push_back(pControlNumbers[nControl]);
					//cout << "Next eff = " << nextEff<< endl;
					//cout << "Eff = " << eff<< endl;
					if (eff + nextEff > maxEff){
						uopts.clear();
						maxEff = eff + nextEff;
						uopts.push_back(pControlNumbers[nControl]);
						efficiency = maxEff;
					}
					nControl ++;
				}
			}
			StageTable.push_back(make_tuple(i,uopts,efficiency));
			//cout << "Time of executing for one state" <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
			
		}
		else StageTable.push_back(make_tuple(i,uopts,0.0));
		
	}
	FullInfo.push_back(StageTable);
	//cout << "before: " << fCount << endl;
	//cout << "after: " << pCount << endl;
	cout << "Stage " << stage << " has " << num << " states." << endl;
	file.close();

}

PossibleControls Model::CheckControls(int stateNum, int tbegin){
	PossibleControls pC = FullControl[stateNum];
	vector <PackageState> s = FullState[stateNum];
	// get free time ends for each node and each number of cores
	map <int, vector <pair <int,int>>> freeTimeEnd, current; // (nodeNum, <(freeCoreNum1, endTime1), (freeCoreNum2, endTime2)...>)
	
	for (int i = 0; i < Nodes.size(); i++){
		vector <pair <int,int>> freeTimesForNode;
		int fullCoreCount = Nodes[i]->GetCoreCount();
		for (int j = 0; j < fullCoreCount; j++)
		{
			freeTimesForNode.push_back(make_pair(j+1, Nodes[i]->GetFreeEnd(j+1,tbegin)));
		}
		freeTimeEnd.insert(make_pair(i+1,freeTimesForNode));
	}

	

	PossibleControls result;
	PossibleControls::iterator it = pC.begin();
	int num = 0;
	// for each controls
	for (; it!= pC.end(); it++ ){
		int flag = true;
		vector <int> busyCores;
		for (int i = 0; i < freeCores.size(); i++)
			busyCores.push_back(0);
		current.clear();
		current = freeTimeEnd;
		for (vector <OnePackageControl>::iterator ip = (*it).begin(); ip!= (*it).end(); ip++){
			float level ;
			int core = (*ip).get<2>();
			int node = (*ip).get<1>();
			if (node !=0) busyCores[node-1]+= core;
			int package = (*ip).get<0>();

			vector <PackageState>::iterator ps  = s.begin();
			for (;ps!=s.end(); ps++)
				if ((*ps).get<0>() == package) level = (*ps).get<3>();
			if (level > 0 && node!=0) {
				int fullCoreCount = Nodes[node-1]->GetCoreCount();
				map<int,vector <pair <int,int>>>::iterator free = current.find(node);
				int minusCount = 0;
			
				for (int j = fullCoreCount-1; j >=0; j--){
					if (minusCount==core) break;
					if (free->second[j].second!=0)
					{
						free->second[j].second=0;
						++minusCount;
							
					}
				}
			}
		}
		// for each package in control
		for (vector <OnePackageControl>::iterator ip = (*it).begin(); ip!= (*it).end(); ip++){
			// check if we haven't enough cores;
			
			float level ;
			int core = (*ip).get<2>();
			int node = (*ip).get<1>();
			if (node !=0) busyCores[node-1]+= core;
			int package = (*ip).get<0>();
			
			vector <PackageState>::iterator ps  = s.begin();
			for (;ps!=s.end(); ps++)
				if ((*ps).get<0>() == package) level = (*ps).get<3>();

			// if we prepare to begin execution 
			if (level == 0 && node !=0){ //uncomment if will crash
			
				int execTime = Packages[package-1]->GetExecTime(node,core);
				int tRealBegin = GetTRealBegin(execTime,level,tbegin);
				map<int,vector <pair <int,int>>>::iterator free = current.find(node);
				vector <pair <int,int>> freePair= free->second;
				for (int i = 0; i < freePair.size(); i++){
					if (core == freePair[i].first){
						// if we don't have enough time
						if (tRealBegin+execTime > freePair[i].second)
						{ flag = false; break;}
						// if we have enough time
						else {
							int fullCoreCount = Nodes[node-1]->GetCoreCount();
							for (int j = fullCoreCount - core; j>=1; j-- ){
								if (freePair[j-1].second > tRealBegin + execTime)
									for (int k = j; k < fullCoreCount; k++){
										//current[node-1][k-1].second = current[node-1][k].second;
										free = current.find(node);
										free->second[k-1].second = free->second[k].second;
									}
							}
							for (int j = fullCoreCount - core + 1; j <= fullCoreCount; j++ ){
								free = current.find(node);
								free->second[j-1].second = 0;
							}
						}
					}
				}
				
			} // if
		}//for
		// uncomment if program will crash
		/*for (int i = 0; i < freeCores.size(); i++){
			if (busyCores[i]>freeCores[i]) flag = false;
		}
*/
		if (flag) {
			result.push_back(*it); 
				//cout << num;
			pControlNumbers.push_back(num);
		}
		num ++;
	}// for
	return result;
}


bool Model::CheckState (int stateNumber){
	vector <int> busyCores;
	for (int i = 0; i < freeCores.size(); i++) busyCores.push_back(0);
	vector<PackageState>::iterator it = FullState[stateNumber].begin();
	for (; it != FullState[stateNumber].end(); it++){
		int nodeNum = (*it).get<1>();
		if (nodeNum!=0) busyCores[nodeNum-1]+=(*it).get<2>();
	}
	for (int i = 0; i < freeCores.size(); i++)
		if (freeCores[i] < busyCores[i]) return false;
	return true;
}



float Model::GetEfficiency(OneControl &it2, int tbegin, int stateNum){
	float e = 0;
	// for each package in control
	for (int i = 0; i < it2.size(); i++){
		int nodeNum = it2[i].get<1>();
		if (nodeNum!=0)
		{
			int packageNum = it2[i].get<0>();
			float level = FullState[stateNum][packageNum-1].get<3>();
			int coreNum = it2[i].get<2>();
			int l = Packages[packageNum-1]->GetExecTime(nodeNum, coreNum);
			int tend;
			if (level == 0){
				tend = tbegin + l;
			}
			else {
				int tRealBegin = GetTRealBegin(l,level,tbegin);
				tend = tRealBegin + l;
			}
			if (tend >= tbegin + delta) e+=EfficiencyByPeriod(coreNum, tbegin, tbegin + delta);
			else e+=EfficiencyByPeriod(coreNum, tbegin, tend);
			if (tend < tbegin){
				cout << tbegin << endl;
				cout << tend << endl;
				cout << l << endl;
				cout << level << endl;
				cout << stateNum << endl;
				for (int i = 0; i < it2.size(); i++) cout << it2[i] << " ";
				cout << endl;
			}
		}
	}
	return e;
}

vector <int> Model::GetUOptsByStateNumber(int stateNum, int tbegin, ofstream &file, float &efficiency){
	
		file << "State " << stateNum << endl;
		vector <int> result;
		
		// taking control variants for this state
		vector <pair<int,float>> eff; // (number of control, efficiency value)
		vector <OneControl> controlVector = FullControl[stateNum];
		// нужно делать проверку

		vector <OneControl>::iterator it2 = controlVector.begin();
		int nControl = 0;
		for (; it2!= controlVector.end(); it2++){
			// if control is avaliable???
			vector<int>::iterator v = find(pControlNumbers.begin(), pControlNumbers.end(), distance(controlVector.begin(), it2)) ; 
			if ( v != pControlNumbers.end() )
			{
				
				float e = GetEfficiency(*it2, tbegin, stateNum);
				//int busyCores = GetBusyCores(*it2);
				//file << busyCores << endl;
				 eff.push_back(make_pair(nControl,e));
				//file << eff.back();
				//file << endl;
			}	
			nControl ++;
		} 	
			sort(eff.begin(), eff.end(), SortPairs);
		
			float maxVal = eff[0].second;
			efficiency = maxVal;
			int i = 0;
			for (int i = 0; i < eff.size(); i++){
				if (eff[i].second == maxVal) 
					result.push_back(eff[i].first);
				file << eff[i].first << " " << eff[i].second << endl;
			}
				
		// if
		
	

		/*while (eff[i].second == maxVal) {
			result.push_back(eff[i].first);
			file << eff[i].first << " " << eff[i].second << endl;
			i++;
			if (i == eff.size()-1) break;
		}*/
		//file.close();
		return result;
}

int Model::GetTRealBegin(int l, float level, int periodBegin){
	float tbegin = periodBegin - l*level;
	for (int t = 0; t <=T; t+=delta){
		if (tbegin >= t && tbegin < t+delta )
			return t;
	}
}

int Model::GetBusyCores(OneControl &control){
	int cores = 0;
	vector <OnePackageControl>::iterator it = control.begin();
	for (;it!= control.end(); it++){
		cores +=(*it).get<2>();
	}
	return cores;
}



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



float Model::Greedy(int currentUopt, float currentEff){
	ofstream f("zhadny.txt");float maxEff = 0.0;
	
	int stateNum = 0;
	int nextState = 0;
	int tbegin = 0;
	for (int i = T/delta-1; i >=0 ; i--){
		int uopt = 0;
		StageInformation &si = FullInfo[i];
		if (i == T/delta-1){
			uopt = currentUopt;
			maxEff+=currentEff;
			nextState = FullNextState[stateNum][uopt];
			
			
		}
		else{
			pControlNumbers.clear();
			float uopteff = 0.0;
			PossibleControls pc = CheckControls(nextState, tbegin);
			for (int j = 0; j <pc.size(); j++){
				
				float eff = GetEfficiency(pc[j], tbegin,nextState);
				if (eff > uopteff){
					uopt = pControlNumbers[j];
					uopteff = eff;
				}
			}
			
			maxEff += uopteff;
			stateNum = nextState;
			nextState = FullNextState[nextState][uopt];
			
		}

		vector <PackageState> p = FullState[stateNum];
		for (int j = 0; j < Packages.size();j++){
			cout << p[j] << " ";
		}
		cout << endl;
		OneControl c = FullControl[stateNum][uopt];
			for (int j = 0; j < Packages.size();j++){
			cout << c[j] << " ";
		}
		cout << endl;
		cout << "Next state num: " << nextState << endl;
		tbegin +=delta;
	}
	
	return maxEff;
}




Model::~Model(void)
{
}
