#include "StdAfx.h"
#include "Model.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

bool SortPairs(pair <int, double> p1, pair <int,double> p2){
	return (p1.second > p2.second);
}

void Model::Init (string resFile, string wfFile, string settingsFile, string xmlFile){
	InitResources(resFile); cout << "Initialization of resources ended" << endl;
	FullInfo.resize(stages);
	InitWorkflows(wfFile); 
	InitSettings(settingsFile);
	xmlBaseName = xmlFile;
}


void Model::InitSettings(string settingsFile){
	try{
		char second[21]; 
		ifstream file(settingsFile, ifstream::in);
		string errOpen = "File " + settingsFile + " was not open";
		string errWrongFormat = "Wrong format in file " + settingsFile + " at line ";
		string errWrongValue = "Wrong value of parameter ";
		string errWrongFormatFull = errWrongFormat;
		string errEarlyEnd = "Unexpected end of file " + settingsFile;
		if (file.fail()) throw UserException(errOpen);
		unsigned int line = 0;
		string s, trim;
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		trim = "canExecuteOnDiffResources=";
		size_t found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		int flag = atoi(s.c_str());
		if (flag==1) canExecuteOnDiffResources = true;
		else if (flag == 0) canExecuteOnDiffResources = false;
		else throw UserException(errWrongValue + "canExecuteOnDiffResources");
		// InputFolderPath="FolderName"
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		trim = "InputFolderPath=\"";
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		trim = "\"";
		found = s.find(trim);
		if (found != s.size()-1) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(found,1);
		path dir = s;
		// DebugInfoFile="debugInfoFileName"
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		trim = "DebugInfoFile=\"";
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		trim = "\"";
		found = s.find(trim);
		if (found != s.size()-1) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(found,1);
		string openErr = " DebugInfoFile cannot be open";
		ex.open(s, ios::app);
		if (ex.fail()) throw UserException(openErr);
		// ResultFile="result.txt"
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		trim = "ResultFile=\"";
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		trim = "\"";
		found = s.find(trim);
		if (found != s.size()-1) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(found,1);
		resFileName = s;
		// T=Tvalue
		getline(file,s);
		++line;
		trim = "T=";
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		T = stoi(s);
		// delta=value
		// T=Tvalue
		getline(file,s);
		++line;
		trim = "delta=";
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		delta = stoi(s);
		stages = T/delta;
		for (int i = 0; i <= T; i+=delta) stageBorders.push_back(i);
		FullInfo.resize(stages);
		// read all filenames from path
		string resourcesFileName;
		vector <string> WFFileNames;
		for (directory_iterator it(dir), end; it != end; ++it) 
		{
			std::cout << "File processed - ";
			std::cout << *it << std::endl;
			string filename = it->path().string();
			if (filename.find("res")==string::npos && filename.find("n")==string::npos) continue;
			if (filename.find("res")!=string::npos )
				resourcesFileName = filename;
			else 
				WFFileNames.push_back(filename);
		}
		InitResources(resourcesFileName);
		for (vector<string>::iterator it = WFFileNames.begin(); it!= WFFileNames.end(); it++)
			InitWorkflows(*it);
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
		if (file.fail()) throw UserException(errOpen);

		string s, trim; int line = 0;
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		trim = "Workflows count = ";
		size_t found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		int workflowsCount = stoi(s);
		vector <int> types; map <pair <int,int>, double> execTime; vector <Package*> pacs; 
		vector <int> cCount; 
		vector <vector <int>> connectMatrix;
		int fullPackagesCount = 0;
		for (int i = 0; i < workflowsCount; i++){
			int packagesCount = 0;
			getline(file,s);
			++line;
			if (file.eof()) throw UserException(errEarlyEnd);
			
			if ((found = s.find("(")) == std::string::npos){
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw UserException(errWrongFormatFull);
			}
			s.erase(0,found+1);
			istringstream iss(s);
			iss >> packagesCount;
			if (iss.fail()) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw UserException(errWrongFormatFull);
			}
			if (packagesCount < 1) {
				sprintf_s(second, "%d", i+1);
				string beginStr = "Workflow ";
				beginStr += second;
				beginStr += " - ";
				beginStr += errPackagesCount;
				throw UserException(beginStr);
			}
			for (int j = 0; j < packagesCount; j++){
				double alpha = 0.0; // part of consequentually executed code
				++fullPackagesCount;
				// Package [packageNumber]
				getline(file,s);
				if (file.eof()) throw UserException(errEarlyEnd);
				++line;
				// Alpha: [alpha value]
				getline(file,s);
				if (file.eof()) throw UserException(errEarlyEnd);
				++line;
				trim = "Alpha: ";
				size_t found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw UserException(errWrongFormatFull);
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				iss >> alpha;
				if (iss.fail()) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw UserException(errWrongFormatFull);
				}
				// Resource types: [resource types values]. -1 means all possible resources
				getline(file,s);
				if (file.eof()) throw UserException(errEarlyEnd);
				++line;
				trim = "Resources types: ";
				found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw UserException(errWrongFormatFull);
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
						throw UserException(errWrongFormatFull);
					}
					types.push_back(typeNumber);
					iss >> comma;
				} while (comma==",");


				
				// Cores count: [cores count values]
				getline(file,s);
				if (file.eof()) throw UserException(errEarlyEnd);
				++line;
				trim = "Cores count: ";
				found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw UserException(errWrongFormatFull);
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
						throw UserException(errWrongFormatFull);
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
					throw UserException(beginStr);
				}
				
				// Computational amount: [amount value]
				long int amount = 0;
				getline(file,s);
				if (file.eof()) throw UserException(errEarlyEnd);
				++line;
				trim = "Computation amount: ";
				found = s.find(trim);
				if (found != 0) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw UserException(errWrongFormatFull);
				}
				s.erase(0,trim.size());
				iss.str(s);
				iss.clear();
				iss >> amount;
				if (iss.fail()) {
					sprintf_s(second, "%d", line);
					errWrongFormatFull += second;
					throw UserException(errWrongFormatFull);
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
			if (file.eof()) throw UserException(errEarlyEnd);
			++line;
			for (int j = 0; j < packagesCount; j++){
				vector <int> row;
				getline(file,s);
				if (file.eof()) throw UserException(errEarlyEnd);
				++line;
				iss.str(s);
				iss.clear();
				for (int k = 0; k < packagesCount; k++){
					int val = 0;
					iss >> val;
					if (iss.fail()) {
						sprintf_s(second, "%d", line);
						errWrongFormatFull += second;
						throw UserException(errWrongFormatFull);
					}
					if (val!=0  && val!=1){
						sprintf_s(second, "%d", i+1);
						string beginStr = "Workflow ";
						beginStr += second;
						beginStr += " - ";
						beginStr += errConnMatrix;
						throw UserException(beginStr);
					}
					row.push_back(val);
				}
				connectMatrix.push_back(row);
			}
			
			Workflows.push_back(new Workflow(pacs,connectMatrix,Workflows.size() + i+1, Resources, typesCores));
			pacs.clear();
			connectMatrix.clear();
		}
		
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Model::InitResources(string f){
	try{
		fullCoresCount = 0;
		map <int, vector<pair <int,int>>> busyIntervals;
		char second[21]; // enough to hold all numbers up to 64-bits
		ifstream file(f.c_str(), ifstream::in);
		string errOpen = "File " + f + " was not open";
		string errEarlyEnd = "Unexpected end of file " + f;
		string errWrongFormat = "Wrong format in file " + f + " at line ";
		string errWrongFormatFull = errWrongFormat;
		string errStageBorders = "InitResources(): stageBorders vector is empty!";
		if (stageBorders.size()==0) throw UserException(errStageBorders);
		if (file.fail()) 
			throw UserException(errOpen);
		string s, trim; int line = 0;
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		trim = "Resources count = ";
		size_t found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		int allResourcesCount = stoi(s);

		trim = "Resources types count = ";
		getline(file,s);
		++line;
		if (file.eof()) throw UserException(errEarlyEnd);
		found = s.find(trim);
		if (found != 0) {
			sprintf_s(second, "%d", line);
			errWrongFormatFull += second;
			throw UserException(errWrongFormatFull);
		}
		s.erase(0,trim.size());
		int typesCount = stoi(s);
		int resourcesCount, coresCount = 0;
		
		for (int i = 0; i < typesCount; i++)
		{
			vector <Resource> typeResources;
			istringstream iss(s);
			getline(file,s);
			++line;
			if (file.eof()) throw UserException(errEarlyEnd);
			sprintf_s(second, "%d", i+1);
			string first = "Type ";
			trim = first + second;
			found = s.find(trim);
			if (found != 0) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw UserException(errWrongFormatFull);
			}
			s.erase(0,trim.size()+2);
			iss.str(s);
			iss.clear();
			iss >> resourcesCount;
			if (iss.fail()) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw UserException(errWrongFormatFull);
			}
			found = s.find(",");
			s.erase(0,found+2);
			iss.str(s);
			iss.clear();
			iss >> coresCount;
			if (iss.fail()) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw UserException(errWrongFormatFull);
			}
			double perf = 0.0;
			getline(file,s);
			trim = "Performance (GFlops): ";
			found = s.find(trim);
			if (found != 0) {
				sprintf_s(second, "%d", line);
				errWrongFormatFull += second;
				throw UserException(errWrongFormatFull);
			}
			s.erase(0,trim.size());
			perf = atof(s.c_str());

			for (int j = 0; j < resourcesCount; j++){
				getline(file,s);
				++line;
				if (file.eof()) throw UserException(errEarlyEnd);
				busyIntervals.clear();
				for (int k = 0; k < coresCount; k++){
					getline(file,s);
					++line;
					if (file.eof()) throw UserException(errEarlyEnd);
					sprintf_s(second, "%d", k+1);
					first = "Core ";
					trim = first + second;
					found = s.find(trim);
					if (found != 0) {
						sprintf_s(second, "%d", line);
						errWrongFormatFull += second;
						throw UserException(errWrongFormatFull);
					}
					s.erase(0,trim.size()+1);
					int diapCount = stoi(s);
					vector<pair<int,int>> oneResDiaps;
					for (int l = 0; l < diapCount; l++){
						if (file.eof()) throw UserException(errEarlyEnd);
						getline(file,s);
						++line;
						iss.str(s);
						iss.clear();
						int one,two;
						iss >> one;
						if (iss.fail()) {
							sprintf_s(second, "%d", line);
							errWrongFormatFull += second;
							throw UserException(errWrongFormatFull);
						}
						iss >> two;
						if (iss.fail()) {
							sprintf_s(second, "%d", line);
							errWrongFormatFull += second;
							throw UserException(errWrongFormatFull);
						}
						oneResDiaps.push_back(make_pair(one,two));
					}
					busyIntervals.insert(make_pair(k+1, oneResDiaps));
				}
				Resource oneRes(j+1,  coresCount, busyIntervals);
				
				typeResources.push_back(oneRes);
			}
			Resources.push_back(new ResourceType(i+1, typeResources, perf));
			fullCoresCount += Resources.back()->GetCoresCount();
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
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

// firstWfNum from ZERO
void Model::StagedScheme(int firstWfNum){
	try{
		vector <int>usedWFNums;
		string wrongFirstNum = "StagedScheme(): wrong firstWFNum";
		if (firstWfNum < 0 || firstWfNum > Workflows.size()-1) throw UserException(wrongFirstNum);
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
				if (i==firstWfNum) {
					currentWfNum = firstWfNum;
					BackBellmanProcedure();
					DirectBellman(i);
					allStagesCores = stagesCores;
					usedWFNums.push_back(firstWfNum);
					/*BellmanToXML(true);
					std::system("pause");*/
					AddDiaps(0, firstWfNum);
					stagesCores.clear();
					/*BellmanToXML(true);
					std::system("pause");*/
				}
				states.clear(); controls.clear(); nextStateNumbers.clear(); fullUsedNums.clear(); stagesCores.clear();
		}
		vector <tuple<int,int,vector<int>>> bestStagesCores;
		while (usedWFNums.size() != Workflows.size()){
			double maxEff = 0.0;
			int bestWfNum = -1;
			for (vector<Workflow*>::size_type i = 0; i < Workflows.size(); i++){
				if (find(usedWFNums.begin(), usedWFNums.end(), i)== usedWFNums.end()){
					currentWfNum = i;
					ReadData(i);
					BackBellmanProcedure();
					double currentEff = DirectBellman(i);
					if (maxEff < currentEff){
						maxEff = currentEff;
						bestWfNum = i;
						bestStagesCores = stagesCores;
					}
					states.clear(); controls.clear(); nextStateNumbers.clear(); fullUsedNums.clear(); stagesCores.clear();
				}
			}
			int diapBegin = allStagesCores.size();
			copy(bestStagesCores.begin(), bestStagesCores.end(),back_inserter(allStagesCores));
			usedWFNums.push_back(bestWfNum);
			usedNums = usedWFNums;
			stagesCores = bestStagesCores;
			currentWfNum = bestWfNum;
			BellmanToXML(true);
			std::system("pause");
			AddDiaps(diapBegin, bestWfNum);
			stagesCores.clear();
			BellmanToXML(true);
			std::system("pause");
		}
		usedNums = usedWFNums;
		for (vector<ResourceType*>::iterator it = Resources.begin(); it!=Resources.end(); it++){
			(*it)->SetInitBusyIntervals();
		}
		stagesCores = allStagesCores;
		
		BellmanToXML(false);

		//int t = clock();
		//cout << "Time of reading data " << (clock()-t)/1000.0 << "sec" << endl;

		
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Model::AddDiaps(int beginIndex, int wfNum){
	int numDiaps = allStagesCores.size() - beginIndex;
	for (int i = beginIndex; i < allStagesCores.size(); i++){
		int package = allStagesCores[i].get_head();
		int stageBegin = allStagesCores[i].get<1>();
		int stageCount = 0;
		vector <int> cores = allStagesCores[i].get<2>();
		int type = GetResourceType(cores[0]);
		int dec = GetResourceTypeBeginIndex(type);
		for (vector<int>::iterator it = cores.begin(); it!= cores.end(); it++)
			*it -= dec;
		cout << "type = " << type << " cores=" << cores.size() << " ";
		double execTime = Workflows[wfNum]->GetExecTime(package, type+1, cores.size());
		if (execTime < delta) stageCount = 1;
		else if ((int)execTime % delta == 0) stageCount = execTime/delta;
		else stageCount = execTime/delta + 1;
		cout << wfNum << " package = " << package << " " << execTime << " "<< stageCount << endl;
		Resources[type]->AddDiaps(stageBegin, stageCount, cores);
	}
}

int Model::GetResourceType(int number){
	try{
		if (number < 0 || number > fullCoresCount-1) throw UserException("GetResourceType(): wrong coreNumber");
		int current = 0;
		for (vector <ResourceType*>::iterator it = Resources.begin(); it!= Resources. end(); it++){
			int currentCoreCount = (*it)->GetCoresCount();
			if (number >=current && number < current + currentCoreCount) return distance(Resources.begin(), it);
			current+=currentCoreCount;
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

int Model::GetResourceTypeBeginIndex(int type){
	try{
		if (type < 0 || type > Resources.size()-1) throw UserException("GetResourceTypeBeginIndex(): wrong typeNumber");
		int current = 0;
		for (int i = 0; i < type; i++)
			current += Resources[i]->GetCoresCount();
		return current;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
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

double Model::DirectBellman(int wfNum){
	try{
		string emptyFileName = "DirectBellman(): empty res file name";
		string errFileOpen = "DirectBellman(): file open error";
		if (resFileName.size()==0) throw UserException(emptyFileName);
		ofstream f(resFileName);
		if (f.fail()) throw UserException(errFileOpen);
		int currentNum = 0;
		double maxEff = FullInfo[0][0].second;
		for (int i = 0; i < stages; i++){
			vector<vector<int>> stageUsedNums;
			timeCore timeCores;
			timeCores.resize(Resources.size());
			int uopt = FullInfo[i][currentNum].first;
			Workflows[wfNum]->PrintState(states[currentNum], f);
			Workflows[wfNum]->PrintControl(controls[currentNum][uopt], f);
			CheckControl(currentNum,uopt,i,timeCores, true, stageUsedNums);
			int fullUsedNumIndex = 0;
			string errMsgIndex = "DirectBellman(): Wrong fullUsedNumIndex value",
				errMsgSize = "DirectBellman() :Wrong fullUsedNums size()";
			for (int j = 0; j < controls[currentNum][uopt].size(); j++){
				if (controls[currentNum][uopt][j] != -1 
					&& Workflows[wfNum]->GetLevel(j,states[currentNum][j])==0){
					if (fullUsedNumIndex > stageUsedNums.size()-1) throw UserException(errMsgIndex);
					if (stageUsedNums.size()==0) throw UserException(errMsgSize);
					stagesCores.push_back(make_tuple(j,i,stageUsedNums[fullUsedNumIndex]));
					fullUsedNumIndex++;
				}
			}
			currentNum = nextStateNumbers[currentNum][uopt];
		}
		return maxEff;
		//BellmanToXML();
		f.close();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Model::BellmanToXML(bool isOne){
	string name = xmlBaseName + "_DP_.jed";
	ofstream f(name);
	MetaXMLInfo(f);
	f << "\t<node_infos>\n";
	BusyToXML(f);
	if (!isOne) StagesCoresToXML(f);
	else 
		StagesCoresToXML(f, currentWfNum);
	f << "\t</node_infos>\n";
	f << "</grid_schedule>\n";
	f.close();
}

void Model::StagesCoresToXML(ofstream&f){
	int usedNumsIndex = 0;
	currentWfNum = usedNums[usedNumsIndex];
	int currentWfPackage = 0;
	for (vector <tuple<int,int,vector<int>>>::size_type i = 0; i < allStagesCores.size(); i++){
		int packageNum = allStagesCores[i].get<0>();
		int tBegin = allStagesCores[i].get<1>() * delta;
		int coresCount = allStagesCores[i].get<2>().size();
		vector <int> cores = allStagesCores[i].get<2>();
		int type = -1;
		int currBeginIndex = 0;
		for (int j = 0; j < Resources.size(); j++){
			if (cores[0] >= currBeginIndex && cores[0] <= currBeginIndex + Resources[j]->GetCoresCount()){
				type = j + 1;
				break;
			}
			currBeginIndex += Resources[j]->GetCoresCount();
		}
		// correct
		double execTime = Workflows[currentWfNum]->GetExecTime(packageNum,type,coresCount);
		int tEnd = tBegin + execTime;
		for (int j = 0; j < cores.size(); j++){
			f << "\t\t<node_statistics>" << endl;
			f << "\t\t	<node_property name=\"id\" value=\"" << packageNum+1 <<"\"/>" << endl;
			f << "\t\t	<node_property name=\"type\" value=\"computation\"/>" << endl;
			f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
			f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
			f << "\t\t	<configuration>" << endl;
			f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
			f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
			f << "\t\t	  <host_lists>" << endl;
			f << "\t\t	    <hosts start=\"" << cores[j] << "\" nb=\"1\"/>" << endl;
			f << "\t\t	  </host_lists>" << endl;
			f << "\t\t	</configuration>" << endl;
			f << "\t\t</node_statistics>" << endl;
		}
		currentWfPackage++;
		if (currentWfPackage == Workflows[currentWfNum]->GetPackageCount()){
			currentWfPackage = 0;
			usedNumsIndex++;
			currentWfNum = usedNums[usedNumsIndex];
		}
	}
}

void Model::StagesCoresToXML(ofstream&f, int currentWfNum){
	int currentWfPackage = 0;
	for (vector <tuple<int,int,vector<int>>>::size_type i = 0; i < stagesCores.size(); i++){
		int packageNum = stagesCores[i].get<0>();
		int tBegin = stagesCores[i].get<1>() * delta;
		int coresCount = stagesCores[i].get<2>().size();
		vector <int> cores = stagesCores[i].get<2>();
		int type = GetResourceType(cores[0]);
		int currBeginIndex = 0;
		
		// correct
		cout << "type=" << type << " cores = " << coresCount << " package = " << packageNum << " ";
		double execTime = Workflows[currentWfNum]->GetExecTime(packageNum,type+1,coresCount);
		cout << execTime << endl;
		int tEnd = tBegin + execTime;
		for (int j = 0; j < cores.size(); j++){
			f << "\t\t<node_statistics>" << endl;
			f << "\t\t	<node_property name=\"id\" value=\"" << packageNum+1 <<"\"/>" << endl;
			f << "\t\t	<node_property name=\"type\" value=\"computation\"/>" << endl;
			f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
			f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
			f << "\t\t	<configuration>" << endl;
			f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
			f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
			f << "\t\t	  <host_lists>" << endl;
			f << "\t\t	    <hosts start=\"" << cores[j] << "\" nb=\"1\"/>" << endl;
			f << "\t\t	  </host_lists>" << endl;
			f << "\t\t	</configuration>" << endl;
			f << "\t\t</node_statistics>" << endl;
		}
		currentWfPackage++;
	}
}


void Model::MetaXMLInfo(ofstream &f){
	int hosts = 0;
	for (vector<Resource*>::size_type i = 0; i < Resources.size(); i++)
		hosts += Resources[i]->GetCoresCount();

	f << "<grid_schedule>\n";
	f << "\t<meta_info>\n";
	f << "\t\t<meta name=\"alloc\" value=\"mcpa\"/>\n";
	f << "\t\t<meta name=\"pack\" value=\"0\"/>\n";
	f << "\t\t<meta name=\"bf\" value=\"0\"/>\n";
	f << "\t\t<meta name=\"ialloc\" value=\"0\"/>\n";
	f << "\t</meta_info>\n";
	f << "\t<grid_info>\n";
	f << "\t\t<info name=\"nb_clusters\" value=\"1\"/>\n";
	f << "\t\t<clusters>\n";
	f << "\t\t  <cluster id=\"0\" hosts=\"" << hosts << "\" first_host=\"0\"/>\n";
	f << "\t\t</clusters>\n";
	f << "\t</grid_info>\n";
}

void Model::BusyToXML(ofstream &f){
	int inc = 0;
	
	for (vector<ResourceType*>::size_type i = 0; i < Resources.size(); i++){
		for (unsigned int j = 0; j < Resources[i]->GetResourceCount(); j++){
			ResourceType *rt = Resources[i];
			Resource r = (*rt)[j];
			map <int,vector<pair<int,int>>> * bI = r.GetBusyIntervals();
			map <int,vector<pair<int,int>>>::iterator bIt = bI->begin();
			for (;bIt != bI->end(); bIt++){
				int coreNum = bIt->first-1;
				coreNum += inc;
				for (vector<pair<int,int>>::size_type k = 0; k < bIt->second.size(); k++){
					int tBegin = bIt->second[k].first;
					int tEnd = bIt->second[k].second;
					f << "\t\t<node_statistics>" << endl;
					f << "\t\t	<node_property name=\"id\" value=\""<< coreNum <<"\"/>" << endl;
					f << "\t\t	<node_property name=\"type\" value=\"busy\"/>" << endl;
					f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
					f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
					f << "\t\t	<configuration>" << endl;
					f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
					f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
					f << "\t\t	  <host_lists>" << endl;
					f << "\t\t	    <hosts start=\"" << coreNum << "\" nb=\"1\"/>" << endl;
					f << "\t\t	  </host_lists>" << endl;
					f << "\t\t	</configuration>" << endl;
					f << "\t\t</node_statistics>" << endl;
				}
			}
			inc += Resources[i]->GetOneResCoresCount();
			
		}
	}
}

void Model::BackBellmanProcedure(){
	for (int i = stages-1; i >=0; i--)
		GetStageInformation(i);
}

// stages are numbered fom zero
void Model::GetStageInformation(int stage){
	int tbegin = stage * delta;
	/*string fname = "stage" + to_string((long long)stage+1) + ".txt";
	ofstream f(fname);*/
	int num = 0; // number of correct states for this stage
	// for all states
	int statesCount = GetStatesCount();
	FullInfo[stage].resize(statesCount);
	for (int i = 0; i < statesCount; i++){
		//f << "State " << i << endl;
		// Workflows[wfNum]->PrintState(states[i],f);
		int uopt = 0;
		double maxEff = 0.0; 
		vector<vector<pair<double, unsigned int>>> timeCoresPerType;
		timeCoresPerType.resize(Resources.size());
		vector<vector<int>> stageUsedNums;
		// if we have right core number for this state
		if (CheckState(i, stage, timeCoresPerType)) {
			vector<vector<int>>::const_iterator controlsIt = controls[i].begin();
			int controlIndex = 0;
			for (; controlsIt!=controls[i].end(); controlsIt++){
				timeCore currentTimeCore = timeCoresPerType;
				if (CheckControl(i, controlIndex, stage, currentTimeCore,false, stageUsedNums)){
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
		//f << uopt << " ";
		//Workflows[0]->PrintControl(controls[i][uopt], f);
		//f << nextStateNumbers[i][uopt] << " ";
		//Workflows[0]->PrintState(states[nextStateNumbers[i][uopt]], f);
		if (stage==0) break;
	}		
	
	//cout << "Stage " << stage << " has " << num << " states." << endl;
	//f.close();

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
	timeCore& timeCoresPerType, bool isUsedNumsNeeded, vector<vector<int>> &stageUsedNums){
	try {
		string errMsg = "timeCoresPerType has wrong size";
		if (timeCoresPerType.size()!=Resources.size()) throw UserException(errMsg);
		Workflows[currentWfNum]->SetTimesCoresForControl(states[state], controls[state][control], timeCoresPerType);
		vector<vector<pair<double, unsigned int>>>::const_iterator tCit = timeCoresPerType.begin();
		unsigned int typeIndex = 0;
		unsigned int inc = 0;
		for (;tCit != timeCoresPerType.end(); tCit++){
			vector <vector<int>> usedNums; 
			if (tCit->size()!=0){
				if (Resources[typeIndex]->Check(*tCit, stage, canExecuteOnDiffResources,
					fullUsedNums, isUsedNumsNeeded, usedNums)==false) 
					return false;
			}
			if (isUsedNumsNeeded){
				for (vector<vector<int>>::iterator packIt = usedNums.begin(); packIt!= usedNums.end(); packIt++){
					for (vector<int>::iterator coreIt = packIt->begin(); coreIt!=packIt->end(); coreIt++)
						*coreIt += inc;
				}
				inc += Resources[typeIndex]->GetCoresCount();
				copy(usedNums.begin(),usedNums.end(), back_inserter(stageUsedNums));
			}
			typeIndex++;	
		}
		return true;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

bool Model::CheckState (const unsigned int state, const unsigned int stage, timeCore& timeCoresPerType){
	vector <vector<int>> stageUsedNums;
	vector <pair<vector<int>,vector<int>>> fullUsedNums;
	Workflows[currentWfNum]->SetTimesCoresForState(states[state], timeCoresPerType);
	vector<vector<pair<double, unsigned int>>>::const_iterator tCit = timeCoresPerType.begin();
	unsigned int typeIndex = 0;
	for (;tCit != timeCoresPerType.end(); tCit++){
		if (tCit->size()!=0){
			if (Resources[typeIndex]->Check(*tCit, stage, canExecuteOnDiffResources, fullUsedNums, false, stageUsedNums)==false) 
				return false;
		}
		typeIndex++;	
	}
	return true;
}


void Model::ReadData(int wfNum){
	try{
		if (wfNum < 0 || wfNum > Workflows.size()-1) throw UserException("ReadData(): wrong wfNum");
		int packages = Workflows[wfNum]->GetPackageCount();
		string name = "wf" + to_string((long long)wfNum+1) + "_controls.txt";
		ifstream f(name);
		if (f.fail()) throw UserException("ReadData(): file " + name + "cannot be open");
		int line = 0;
		string errEarlyEnd = "ReadData(): early file end";
		string s;
		getline(f,s);
		line++;
		if (f.eof()) throw UserException(errEarlyEnd);
		if (s.find("State")==string::npos) throw UserException("ReadData(): wrong format at line " + to_string((long long)line));
		while (!f.eof()){
			// read state
			getline(f,s);
			line++;
			if (f.eof()) throw UserException(errEarlyEnd);
			istringstream iss(s);
			vector <int> state;
			for (int i = 0; i < packages; i++){
				iss >> s;
				state.push_back(stoi(s));
			}
			states.push_back(state);
			getline(f,s);
			line++;
			if (f.eof()) throw UserException(errEarlyEnd);
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
				oneStateControls.push_back(control);

				getline(f,s);
				line++;
				if (f.eof()) throw UserException(errEarlyEnd);
				nextStates.push_back(stoi(s));

				getline(f,s);
				line++;

			} while (s.find("State")==string::npos && !f.eof());
			controls.push_back(oneStateControls);
			nextStateNumbers.push_back(nextStates);
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

Model::~Model(void)
{
}
