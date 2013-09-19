#include "StdAfx.h"
#include "Package.h"


Package::Package(int u, vector <int> &r, vector <int> &c, map <pair <int,int>, float> &e)
{
	uid = u;
	resourceTypes = r;
	coreCounts = c;
	execTime = e;
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

Package::~Package(void)
{
}
