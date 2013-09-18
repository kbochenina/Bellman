#pragma once
class Package
{
	int uid;
	vector <int> nodeNumbers; // numbers of nodes where package can be executed
	vector <int> coreCounts; // numbers of cores where package can be executed 
	map <pair <int,int>, int> execTime; // ((1,2,15)) means that package executes 15 seconds on 2 cores of node 1
	
public:
	Package(int u, vector <int> &n, vector <int> &c, map <pair <int,int>, int> &e);
	vector <int>& GetNodeNumbers() {return nodeNumbers;}
	vector <int>& GetCoreCounts() {return coreCounts;}
	int GetExecTime (int nodeNum, int coreNum){
		map <pair <int,int>, int>::iterator i = execTime.find(make_pair(nodeNum, coreNum));
		if (i == execTime.end()) cout << "Package" << uid << " node "<< nodeNum << " core" << coreNum << endl;
		return (*i).second;
	}
	
	~Package(void);
};

