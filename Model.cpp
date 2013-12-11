#include "StdAfx.h"
#include "Model.h"
#include <boost/filesystem.hpp>


using namespace boost::filesystem;

extern bool directBellman;

typedef tuples::tuple<int, int, int, double> mytuple;
 
bool effDecr (const mytuple &lhs, const mytuple &rhs){
  return get<3>(lhs) > get<3>(rhs);
}

bool SortPairs(pair <int, double> p1, pair <int,double> p2){
	return (p1.second > p2.second);
}

void Model::Init (string resFile, string wfFile, string settingsFile, string xmlFile){
	InitResources(resFile); 
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

void Model::SetData(){
	for (unsigned int i = 0; i < Workflows.size(); i++){
		Workflows[i]->SetIsPackageInit();
		Workflows[i]->SetPackagesStates();
		//Workflows[i]->PrintExecTime();
		int t = clock();
		Workflows[i]->SetFullPackagesStates(0, states, controls, nextStateNumbers);
		for (int j = 0; j < stages; j++) 
			FullInfo[j].resize(states.size());
		//cout << "Time of SetFullPackagesStates() " << (clock()-t)/1000.0 << endl;
		//t = clock();
		Workflows[i]->PrintPackagesStates(states);
		//cout << "Time of PrintPackagesStates() " << (clock()-t)/1000.0 << endl;
		Workflows[i]->PrintControls(states,controls, nextStateNumbers);
		//cout << "Time of PrintControls() " << (clock()-t)/1000.0 << endl;
		states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
	}
}

// firstWfNum from ZERO
void Model::StagedScheme(int firstWfNum){
	try{
		xmlBaseName = "DP_";
		double stagedT = clock();
		string resFileName = "staged_scheme_" + to_string((long long)firstWfNum) + ".txt";
		ofstream res(resFileName);
		if (res.fail()) 
			throw UserException("Unable to create res file");
		res << "Attempt: " << firstWfNum << endl;
		cout << "Attempt: " << firstWfNum << endl;
		vector <int>usedWFNums;
		string wrongFirstNum = "StagedScheme(): wrong firstWFNum";
		vector<double> eff;
	
		if (firstWfNum < 0 || firstWfNum > Workflows.size()-1) throw UserException(wrongFirstNum);
		cout << "stage 1" << endl;
		cout << "firstWfnum = " << firstWfNum << " ";
		double s = clock();
		ReadData(firstWfNum);
		currentWfNum = firstWfNum;
		directBellman = false;
		BackBellmanProcedure();
		directBellman = true;
		eff.push_back(DirectBellman(firstWfNum));
		cout << "elapsed time: " << (clock()-s)/1000.0 << " sec" << endl;
		allStagesCores = stagesCores;
		usedWFNums.push_back(firstWfNum);
		BellmanToXML(true);
		//std::system("pause");
		stagesCores.clear();
		FixNewBusyIntervals();
		BellmanToXML(true);
		//std::system("pause");
		
		states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
		
		res << "WF " << firstWfNum << endl;
		for (vector <tuples::tuple<int,int,vector<int>>>::iterator it = allStagesCores.begin(); it!= allStagesCores.end(); it++){
			res << "(" << it->get<0>() << " " << it->get<1>() << " (";
			for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
				res << *it2 << " ";
			res << "))";
		}
		res << endl;
		vector <vector <busyIntervals>> bestBusyIntervals;
		vector <tuples::tuple<int,int,vector<int>>> bestStagesCores;
		while (usedWFNums.size() != Workflows.size()){
			cout << "stage " << usedWFNums.size() + 1 << endl;
			double maxEff = 0.0;
			int bestWfNum = -1;
			for (vector<Workflow*>::size_type i = 0; i < Workflows.size(); i++){
				if (find(usedWFNums.begin(), usedWFNums.end(), i)== usedWFNums.end()){
					currentWfNum = i;
					double s = clock();
					cout << "currentWfNum = " << currentWfNum << " ";
					ReadData(i);
					directBellman = false;
					BackBellmanProcedure();
					directBellman = true;
					double currentEff = DirectBellman(i);
					cout << "elapsed time: " << (clock()-s)/1000.0 << " sec" << endl;
					if (maxEff < currentEff){
						maxEff = currentEff;
						bestWfNum = i;
						bestStagesCores = stagesCores;
						bestBusyIntervals.clear();
						GetBestBusyIntervals(bestBusyIntervals);
					}
					states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
				}
			}
			int diapBegin = allStagesCores.size();
			copy(bestStagesCores.begin(), bestStagesCores.end(),back_inserter(allStagesCores));
			usedWFNums.push_back(bestWfNum);
			usedNums = usedWFNums;
			stagesCores = bestStagesCores;
			currentWfNum = bestWfNum;
			eff.push_back(maxEff);
			res << "WF " << bestWfNum << endl;
			for (vector <tuples::tuple<int,int,vector<int>>>::iterator it = bestStagesCores.begin(); it!= bestStagesCores.end(); it++){
				res << "(" << it->get<0>() << " " << it->get<1>() << " (";
				for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
					res << *it2 << " ";
				res << "))";
			}
			res << endl;
			BellmanToXML(true);
			//std::system("pause");
			stagesCores.clear();
			SetBestBusyIntervals(bestBusyIntervals);
			FixNewBusyIntervals();
			BellmanToXML(true);
			//std::system("pause");
		}
		usedNums = usedWFNums;
		SetFirstBusyIntervals();
		stagesCores = allStagesCores;
		BellmanToXML(false);
		
		res << "Attempt " << firstWfNum << endl << "Workflow order: " ;
		for (vector<int>::size_type i = 0; i < usedNums.size(); i++){
			res << usedNums[i] << " ";
		}
		res << endl << "Efficiencies: " ;
		double sum = 0.0;
		for (vector<int>::size_type i = 0; i < eff.size(); i++){
			sum+=eff[i];
			res << eff[i] << " ";
		}
		res << endl << "Max eff: " << sum << endl << endl;
		SetFirstBusyIntervals();
		states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
		res.close();
		cout << "Max eff: " << sum << endl;
		cout << "Elapsed time: " << (clock()-stagedT)/1000.0 << " sec" << endl;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}



double Model::Greedy(int wfNum){
	double eff = 0.0;
	xmlBaseName = "Greedy_";
	currentWfNum = wfNum;
	vector<vector <int>> ready, exec, possible;

	ready.resize(stages);
	exec.resize(stages);
	possible.resize(stages);

	int packageCount = Workflows[wfNum]->GetPackageCount();

	for (int i = 0; i < stages; i++){
		cout << "\nstage " << i+1 << endl;
		// 1. Get possible packages numbers
		if (i == 0){
			for (int j = 0; j < packageCount; j++){
				if (Workflows[wfNum]->IsPackageInit(j))
					possible[i].push_back(j);
			}
		}
		else {
			for (int j = 0; j < packageCount; j++){
				vector <vector <int>>::iterator it = ready.begin() + i;
				// if package j is not ready
				if (find(it->begin(), it->end(),j) == it->end()){
					it = exec.begin() + i;
					// and package j is not executed
					if (find(it->begin(), it->end(),j) == it->end()){
						vector<int> depend;
						// depend will contain packages from which j depends
						Workflows[wfNum]->GetDependency(j, depend);
						bool isPossible = true;
						vector <int>::iterator depIt = depend.begin();
						it = ready.begin() + i;
						// if one of those packages are not ready, j cannot begin execution
						for (; depIt!=depend.end(); depIt++){
							if (find(it->begin(), it->end(),*depIt) == it->end())
								isPossible = false;
						}
						if (isPossible) 
							possible[i].push_back(j);
					}
				}
			}
		}
		cout << "Possible: ";
		for (vector <int>::iterator it = possible[i].begin(); it!= possible[i].end(); it++)
			cout << *it << " ";

		// 2. Get tuples (packageNum, type, coresNum, efficiency)
		vector <tuples::tuple<int, int, int, double>> variants;
		vector <int>::iterator it = possible[i].begin();
		for (; it!= possible[i].end(); it++){
			// type + core + execTime
			map <pair <int,int>, double> out;
			Workflows[wfNum]->GetExecTime(out,*it);
			map <pair <int,int>, double>::iterator outIt = out.begin();
			for (; outIt!=out.end(); outIt++){
				int tend = i*delta + outIt->second;
				if (tend > T) tend = T;
				double eff = EfficiencyByPeriod(outIt->first.second,i*delta,tend);
				variants.push_back(tuples::make_tuple(*it, outIt->first.first, outIt->first.second, eff));
			}
		}
		// 3. Sort variants by execTime decrease
		sort(variants.begin(), variants.end(), effDecr);
		// find a concretizations
		vector <mytuple>::const_iterator t = variants.begin();
		vector <int> packagesConcreted;
		for (;t!= variants.end(); t++){
			int packageIndex =  t->get<0>();
			if (find(packagesConcreted.begin(), packagesConcreted.end(), packageIndex) == packagesConcreted.end()){
				// execTime + coreNum
				vector <vector<int>> oneTypeCoreNums;
				vector<pair<double, unsigned int>> tC;
				int typeIndex = t->get<1>()-1;
				double execTime = Workflows[wfNum]->GetExecTime(packageIndex, t->get<1>(), t->get<2>());
				tC.push_back(make_pair(execTime, t->get<2>()));
				bool checkType = Resources[typeIndex]->Check(tC, i, oneTypeCoreNums, false);
				// if we can not find a concretization, we'll proceed next variant
				if (!checkType)
					continue;
				eff +=  t->get<3>();
				packagesConcreted.push_back(packageIndex);
				Resources[typeIndex]->AddDiaps(i,execTime, oneTypeCoreNums[0]);
				// get global core numbers
				CoresLocalToGlobal(typeIndex, oneTypeCoreNums[0]);
				// add result to stagesCores
				stagesCores.push_back(tuples::make_tuple(packageIndex,i, oneTypeCoreNums[0]));
				// push package number to executed on required number stages
				int last = i + execTime/delta + 1;
				if ( (int)execTime % delta == 0) last++;
				for (int j = i; j < last; j++)
					exec[j].push_back(packageIndex);
				// push package number to ready
				for (int j = last; j < stages; j++)
					ready[j].push_back(packageIndex);
			}
		}
		cout << "\nReady: ";
		for (vector <int>::iterator it = ready[i].begin(); it!= ready[i].end(); it++)
			cout << *it << " ";
		cout << endl << "Executed: ";
		for (vector <int>::iterator it = exec[i].begin(); it!= exec[i].end(); it++)
			cout << *it << " ";

	}
	BellmanToXML(true);
	return eff;
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
		vector <int> bestStates;
		vector <int> bestControlNums;
		int currentNum = 0;
		double maxEff = FullInfo[0][0].second;
		for (int i = 0; i < stages; i++){
			bestStates.push_back(currentNum);
			int uopt = FullInfo[i][currentNum].first;
			bestControlNums.push_back(uopt);
			Workflows[wfNum]->PrintState(states[currentNum], f);
			Workflows[wfNum]->PrintControl(controls[currentNum][uopt], f);
			currentNum = nextStateNumbers[currentNum][uopt];
		}

		
		timeCore timeCores;
		timeCores.resize(Resources.size());
		
		vector <vector<int>> packagesCoresNums;
		packagesCoresNums.resize(Workflows[currentWfNum]->GetPackageCount());
		vector<int> proceededPNums;
		proceededPNums.resize(Workflows[currentWfNum]->GetPackageCount(),-1);

		for (int i = stages-1; i >=0; i--){
			if (!FindConcretization(bestStates[i],-1,i,timeCores,packagesCoresNums))
				throw UserException("DirectBellman() awful error: cannot find the concretization for state");
			for (auto it = packagesCoresNums.begin(); it!= packagesCoresNums.end(); it++){
				if (it->size()!=0){
					int pNum = std::distance(packagesCoresNums.begin(), it);
					if (find(proceededPNums.begin(), proceededPNums.end(),pNum)==proceededPNums.end()){
						proceededPNums.push_back(pNum);
						double execTime = Workflows[currentWfNum]->GetExecTime(pNum, states[bestStates[i]][pNum]);
						int stageBegin = i+1 - execTime/delta;
						if ((int)execTime%delta==0) stageBegin++;
						stagesCores.push_back(tuples::make_tuple(pNum, stageBegin, *it));
					}
				}
			}

			timeCores.clear();
			timeCores.resize(Resources.size());
			if (!FindConcretization(bestStates[i],bestControlNums[i],i,timeCores,packagesCoresNums))
				throw UserException("DirectBellman() awful error: cannot find the concretization for control");
			for (auto it = packagesCoresNums.begin(); it!= packagesCoresNums.end(); it++){
				if (it->size()!=0){
					int pNum = std::distance(packagesCoresNums.begin(), it);
					if (find(proceededPNums.begin(), proceededPNums.end(),pNum)==proceededPNums.end()){
						proceededPNums.push_back(pNum);
						stagesCores.push_back(tuples::make_tuple(pNum, i, *it));
					}
				}
			}
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
	string name = xmlBaseName + to_string((long long)outputFileNumber++) + ".jed";
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
	try{
		int usedNumsIndex = 0;
		if (usedNums.size()==0)
			throw UserException("Model::StagesCoresToXML() : usedNums.size()==0");
		currentWfNum = usedNums[usedNumsIndex];
		int currentWfPackage = 0;
		for (vector <tuples::tuple<int,int,vector<int>>>::size_type i = 0; i < allStagesCores.size(); i++){
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
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Model::StagesCoresToXML(ofstream&f, int currentWfNum){
	int currentWfPackage = 0;
	for (vector <tuples::tuple<int,int,vector<int>>>::size_type i = 0; i < stagesCores.size(); i++){
		int packageNum = stagesCores[i].get<0>();
		int tBegin = stagesCores[i].get<1>() * delta;
		int coresCount = stagesCores[i].get<2>().size();
		vector <int> cores = stagesCores[i].get<2>();
		int type = GetResourceType(cores[0]);
		int currBeginIndex = 0;
		// correct
		//cout << "type=" << type << " cores = " << coresCount << " package = " << packageNum << " ";
		double execTime = Workflows[currentWfNum]->GetExecTime(packageNum,type+1,coresCount);
		//cout << execTime << endl;
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

void Model::GetBestBusyIntervals(vector<vector<busyIntervals>> & out){
	out.resize(Resources.size());
	for (vector<Resource*>::size_type res = 0; res < Resources.size(); res++){
		Resources[res]->GetCurrentBusyIntervals(out[res]);
	}
}

void Model::SetBestBusyIntervals(vector<vector<busyIntervals>> & in){
	for (vector<Resource*>::size_type res = 0; res < Resources.size(); res++){
		Resources[res]->SetCurrentBusyIntervals(in[res]);
	}
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
	//string fname = "stage" + to_string((long long)stage+1) + ".txt";
	//ofstream f(fname);
	bool debugFlag = false;
	int num = 0; // number of correct states for this stage
	// for all states
	int statesCount = GetStatesCount();
	FullInfo[stage].resize(statesCount);
	for (int i = 0; i < statesCount; i++){
		/*Workflows[currentWfNum]->PrintState(states[i],f);*/
		int uopt = 0;
		double maxEff = 0.0; 
		timeCore stateTimeCores;
		stateTimeCores.resize(Resources.size());
		vector <vector<int>> stateCoresNums; // ((coreNum1, coreNum2,...)-package1, (coreNum1, coreNum2,...)-package2)
		stateCoresNums.resize(Workflows[currentWfNum]->GetPackageCount());
		// if we have right core number for this state
		if (FindConcretization(i, -1, stage, stateTimeCores, stateCoresNums)) {
			vector <vector<int>> stateControlCoresNums = stateCoresNums;
			vector<vector<int>>::const_iterator controlsIt = controls[i].begin();
			int controlIndex = 0;
			for (; controlsIt!=controls[i].end(); controlsIt++){

				timeCore controlTimeCores;
				controlTimeCores.resize(Resources.size());
				if (FindConcretization(i, controlIndex, stage, controlTimeCores,stateControlCoresNums)){
					// if it is the last period
					double currEff = GetEfficiency(stage, stateTimeCores, controlTimeCores);
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
				ResetBusyIntervals();
			}
			stateCoresNums.clear();
		}
		FullInfo[stage][i] = make_pair(uopt, maxEff);
		/*f << uopt << " ";
		Workflows[currentWfNum]->PrintControl(controls[i][uopt], f);
		f << nextStateNumbers[i][uopt] << " ";
		Workflows[currentWfNum]->PrintState(states[nextStateNumbers[i][uopt]], f);*/
		if (stage==0) break;
	}		
	
	//cout << "Stage " << stage << " finished" << endl;
//	f.close();
	
}


void Model::ResetBusyIntervals(){
	for (auto i = Resources.begin(); i!= Resources.end(); i++){
		(*i)->SetInitBusyIntervals();
	}
}

void Model::FixNewBusyIntervals(){
	for (auto i = Resources.begin(); i!= Resources.end(); i++){
		(*i)->FixBusyIntervals();
	}
}

void Model::SetFirstBusyIntervals(){
	for (auto i = Resources.begin(); i!= Resources.end(); i++){
		(*i)->SetFirstBusyIntervals();
	}
}

double Model::GetEfficiency(const int & stage, const timeCore& state, const timeCore& control){
	double eff = 0.0;
	timeCore::const_iterator tcIt = state.begin();
	for (;tcIt!=state.end(); tcIt++){
		vector<pair<double,unsigned int>>::const_iterator pairsIt = tcIt->begin();
		for (;pairsIt!=tcIt->end(); pairsIt++){
			int tbegin = stage*delta;
			int tend = tbegin + (int)pairsIt->first;
			if (tend > T) tend = T;
			eff += EfficiencyByPeriod(pairsIt->second, tbegin, tend);
		}
	}
	tcIt = control.begin();
	for (;tcIt!=control.end(); tcIt++){
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

void Model::SetOneTypeCoreNums(int typeIndex, vector<int> &addForbiddenCoreNums, vector<int>& out){
	try{
		if (typeIndex > Resources.size()-1 || typeIndex < 0) throw UserException("Model::SetOneTypeCoreNums() : wrong typeIndex value");
		// find the first and last global indexes of cores of type typeIndex
		int beginCoreIndex = 0, lastCoreIndex = Resources[0]->GetCoresCount()-1;
		for (int i = 1; i <= typeIndex; i++){
			beginCoreIndex = lastCoreIndex+1;
			lastCoreIndex = beginCoreIndex + Resources[i]->GetCoresCount()-1;
		}
		/*
			if state package executes on resource of this type, copy busy resources to oneTypeCoreNums 
		*/
		for (vector<int>::iterator aI = addForbiddenCoreNums.begin(); aI!= addForbiddenCoreNums.end(); aI++){
			
				if (*aI >= beginCoreIndex && *aI <= lastCoreIndex){
						out.push_back(*aI);
				}
			
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Model::CoresLocalToGlobal(int resType, vector <int> & cores){
	try{
		if (resType < 0 || resType > Resources.size() -1 )
			throw UserException("Model::CoresLocalToGlobal() : wrong resType");
		if (cores.size()==0)
			throw UserException("Model::CoresLocalToGlobal() : cores.size() == 0");
		int inc = 0;
		for (int i = 0; i < resType; i++)
			inc += Resources[i]->GetCoresCount();
		vector<int>::iterator it = cores.begin();
		for (;it!=cores.end(); it++)
			*it += inc;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}


/* parameters:
state - state number in states
control - control number. If value = -1, we find a concretization for state
stage - current stage number, counted from 0
timeCoresPerType - array of resources types size, each element is a vector of <execTime, numCores> for this type
				   (received from current state in function SetTimesCoresForState())
				   Later timeCoresPerType contains PARTICLE times for states (only on current stage)
					
packagesCoresNums - out parameter, vector of concrete GLOBAL coreNums after concretising the state
*/
bool Model::FindConcretization (const unsigned int &state, const int &control, const unsigned int &stage, 
	timeCore& timeCoresPerType, vector <vector<int>>& packagesCoresNums){
	try{
		if (timeCoresPerType.size()!=Resources.size()) throw UserException("Model::CheckState() : timeCoresPerType has wrong size");
		if (packagesCoresNums.size()==0) throw UserException("Model::CheckState() : packagesCoresNums has zero size");
		// number of packages concretized earlier
		vector <int> oldPNums; 
		for (auto it = packagesCoresNums.begin(); it!= packagesCoresNums.end(); it++){
			if (it->size()!=0) oldPNums.push_back(distance(packagesCoresNums.begin(), it));
		}
		vector <vector<int>> packagesIndexesPerType;
		packagesIndexesPerType.resize(Resources.size());
		
		bool isStateConcretized = false;

		if (control == -1){
			Workflows[currentWfNum]->SetTimesCoresForState(states[state], timeCoresPerType, packagesIndexesPerType);
			isStateConcretized = true;
		}
		else 
			Workflows[currentWfNum]->SetTimesCoresForControl(states[state], controls[state][control], timeCoresPerType,
			packagesIndexesPerType);

		for (vector<vector<int>>::iterator it = packagesCoresNums.begin(); it!= packagesCoresNums.end(); it++){
			// if package also has the concretization
			if (it->size()!=0){
				// find the package number
				int pNum = distance(packagesCoresNums.begin(), it);
				// if this package executed on this state
				// delete his parameters from input arrays timeCoresPerType and packagesIndexesPerType
				for (auto pI = packagesIndexesPerType.begin(); pI!= packagesIndexesPerType.end(); pI++){
					int type = distance(packagesIndexesPerType.begin(), pI);
					auto findIt = find(pI->begin(),pI->end(), pNum);
					if (findIt!=pI->end()){
						int index = distance(pI->begin(), findIt);
						timeCoresPerType[type].erase(timeCoresPerType[type].begin()+index,
							timeCoresPerType[type].begin()+index+1);
						packagesIndexesPerType[type].erase(packagesIndexesPerType[type].begin()+index,
							packagesIndexesPerType[type].begin()+index+1);
					}
				}
			}
		}

		vector<vector<pair<double, unsigned int>>>::iterator tCit = timeCoresPerType.begin();
		unsigned int typeIndex = 0; 
		unsigned int inc = 0;
		for (;tCit != timeCoresPerType.end(); tCit++){
			if (tCit->size()!=0){
				vector <vector<int>> oneTypeCoreNums;
				// oneTypeCoreNums contains LOCAL core numbers for type typeindex
				bool checkType = Resources[typeIndex]->Check(*tCit, stage, oneTypeCoreNums, isStateConcretized);
				if (checkType == false) return checkType;
				int packageIndex = 0;
				// for each packages indexes for this type
				for (vector<int>::iterator indexIt = packagesIndexesPerType[typeIndex].begin();
					indexIt!=packagesIndexesPerType[typeIndex].end(); indexIt++){
					auto coresIt = std::begin(oneTypeCoreNums[packageIndex]);
					// find global core numbers
					//CoresLocalToGlobal(typeIndex, oneTypeCoreNums[packageIndex]);
					for (; coresIt!= std::end(oneTypeCoreNums[packageIndex]); coresIt++)
						*coresIt += inc;
					// concretizing the package
					packagesCoresNums[*indexIt] = oneTypeCoreNums[packageIndex];
					packageIndex++;
				}
			}
			inc += Resources[typeIndex]->GetCoresCount();
			typeIndex++;	
		}

		// add diaps for new concretized packages
		for (auto it = packagesCoresNums.begin(); it!= packagesCoresNums.end(); it++){
			int pNum = distance(packagesCoresNums.begin(), it);
			// if package was concretized now
			if (it->size()!=0 && find(oldPNums.begin(), oldPNums.end(),pNum)==oldPNums.end()){
				// if we concretize a state
				int stageBegin = -1;
				double execTime = -1;
				int type = -1;
				if (control  == -1){
					execTime = Workflows[currentWfNum]->GetExecTime(pNum, states[state][pNum]);
					stageBegin = stage - (int)execTime/delta;
					if ((int)execTime%delta==0) {
						stageBegin++;
					}
					type = GetResourceType((*it)[0]);
					
				}
				else {
					int c = controls[state][control][pNum];
					if (c > typesCores.size()-1) throw UserException("FindConcretization() : wrong control value");
					type = typesCores[c].first-1;
					int cores = typesCores[c].second;
					execTime = Workflows[currentWfNum]->GetExecTime(pNum, type + 1, cores);
					stageBegin = stage;
				}
				int stageCount = execTime/delta + 1;
				vector <int> localCores;
				int dec = GetResourceTypeBeginIndex(type);
				for (auto coresIt = it->begin(); coresIt != it->end(); coresIt++){
					localCores.push_back(*coresIt-dec);
				}
				Resources[type]->AddDiaps(stageBegin,execTime, localCores);
			}
		}

		tCit = timeCoresPerType.begin();
		// find the PARTICLE times for states/controls (times only on current stage)
		for (;tCit != timeCoresPerType.end(); tCit++){
			if (tCit->size()!=0){
				for (vector<pair<double, unsigned int>>::iterator pairsIt = tCit->begin(); pairsIt!=tCit->end(); pairsIt++){
					if (pairsIt->first > delta){
						if (control == -1){
							int fullBusyStages = pairsIt->first/delta;
							pairsIt->first -= fullBusyStages*delta;
						}
						else {
							pairsIt->first = delta;
						}
					}
				}
			}
		}
		return true;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
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
