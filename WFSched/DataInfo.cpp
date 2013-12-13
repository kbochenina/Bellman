#include "StdAfx.h"
#include "DataInfo.h"
#include <fstream>
#include <sstream> // istringstream
#include <string>
#include <iostream>
#include <boost/filesystem.hpp> // directory_iterator, path
#include <iterator>
#include "UserException.h"

using namespace boost::filesystem; // directory_iterator, path

DataInfo::~DataInfo(void)
{
}

DataInfo::DataInfo( string fSettings )
{
	Init(fSettings);
}

void DataInfo::Init(string settingsFile){
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
		int T = 0, delta = 0;
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
		bool canExecuteOnDiffResources;
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
		ofstream ex;
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
		string resFileName = s;
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
		context.SetContext(T, delta);	
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
		InitResources(resourcesFileName, canExecuteOnDiffResources);
		for (vector<string>::iterator it = WFFileNames.begin(); it!= WFFileNames.end(); it++)
			InitWorkflows(*it);
		ex.close();
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

void DataInfo::InitWorkflows(string f){
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
		vector <int> types; map <pair <int,int>, double> execTime; vector <Package> pacs; 
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
						for (unsigned int i = 0; i < resources.size(); i++)
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
						if (resources[k].GetCoresCount() < cCount[l]) break; 
						// Amdal's law
						double acc = (double) 1.00 / (alpha + (1-alpha)/(l+1));
						// execTime = amount / (perf * acc)
						double exTime = amount / (resources[k].GetPerf() * acc);
						execTime.insert(make_pair(make_pair(types[k], cCount[l]), exTime));
					}
				}
				Package p(fullPackagesCount,types,cCount,execTime);
				pacs.push_back(p);
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
			Workflow w(workflows.size() + i+1, pacs,connectMatrix);
			workflows.push_back(w);
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

void DataInfo::InitResources(string f, bool canExecuteOnDiffResources){
	try{
		fullCoresCount = 0;
		map <int, vector<pair <int,int>>> busyIntervals;
		char second[21]; // enough to hold all numbers up to 64-bits
		ifstream file(f.c_str(), ifstream::in);
		string errOpen = "File " + f + " was not open";
		string errEarlyEnd = "Unexpected end of file " + f;
		string errWrongFormat = "Wrong format in file " + f + " at line ";
		string errWrongFormatFull = errWrongFormat;
		//string errStageBorders = "InitResources(): stageBorders vector is empty!";
		//if (stageBorders.size()==0) throw UserException(errStageBorders);
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
		int resourcesCount = 0, coresCount = 0;

		for (int i = 0; i < typesCount; i++)
		{
			vector <BusyIntervals> typeBI;
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
				// add busyIntervals for current resource to a vector <BusyIntervals>
				typeBI.push_back(busyIntervals);
			}
			ResourceType r(i+1,resourcesCount, coresCount, perf, typeBI, canExecuteOnDiffResources, context);
			resources.push_back(r);
			for (int j = 1; j <= coresCount; j++)
				typesCores.push_back(make_pair(i+1, j));
			fullCoresCount += coresCount * resourcesCount;
		}

		/*int initVal = 0;
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
		;*/
		file.close();
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void DataInfo::FixBusyIntervals(){
	for (auto i = resources.begin(); i!= resources.end(); i++){
		i->FixBusyIntervals();
	}
}

void DataInfo::ResetBusyIntervals(){
	for (auto i = resources.begin(); i!= resources.end(); i++){
		i->ResetBusyIntervals();
	}
}

void DataInfo::SetInitBusyIntervals(){
	for (auto i = resources.begin(); i!= resources.end(); i++){
		i->SetInitBusyIntervals();
	}
}

void DataInfo::GetCurrentIntervals(vector<vector<BusyIntervals>> &out){
	out.resize(resources.size());
	for (vector<ResourceType>::size_type res = 0; res < resources.size(); res++){
		resources[res].GetCurrentIntervals(out[res]);
	}
}

void DataInfo::SetCurrentIntervals(vector<vector<BusyIntervals>> &out){
	out.resize(resources.size());
	for (vector<ResourceType>::size_type res = 0; res < resources.size(); res++){
		resources[res].SetCurrentIntervals(out[res]);
	}
}

int DataInfo::GetResourceType(int number){
	try{
		if (number < 0 || number > fullCoresCount-1) 
			throw UserException("DataInfo::GetResourceType() error. Wrong coreNumber");
		int current = 0;
		for (vector <ResourceType>::iterator it = resources.begin(); it!= resources.end(); it++){
			int currentCoreCount = it->GetCoresCount();
			if (number >= current && number < current + currentCoreCount) 
				return distance(resources.begin(), it);
			current+=currentCoreCount;
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

// get initial core index of resource type
int DataInfo::GetInitResourceTypeIndex(int type){
	int index = 0;
	for (int i = 0; i < type; i++)
		index += resources[i].GetCoresCount();
	return index;
}

// do the same
int DataInfo::GetTypeCoreIndex(const pair<int,int>& typeCore){
	auto it = find(typesCores.begin(), typesCores.end(), typeCore);
	if (it == typesCores.end())
		return -2;
	else return distance(typesCores.begin(), it);
}