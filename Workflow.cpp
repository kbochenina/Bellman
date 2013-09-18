#include "stdafx.h"
#include "Workflow.h"


Workflow::Workflow (std::vector <Package*> p, std::vector <std::vector <int>> c){
	packages = p;
	connectMatrix = c;
}



bool Workflow::IsDepends(int one, int two){
	if (connectMatrix[one][two]==1) return true;
	else {
		vector <int> notDirectDepends;
		for (int i = 0; i < connectMatrix[one].size(); i++){
			if (connectMatrix[one][i]!=0) 
				if (IsDepends(i,two)) return true;
		}
		return false;
		
	}
}

void Workflow::GetDependency(int pNum, vector<int>&res){
	for (int i = 0; i < connectMatrix[pNum].size(); i++){
		if (connectMatrix[i][pNum]==1) res.push_back(i);
	}
}

bool Workflow::IsPackageInit(int pNum){
	bool flag = true;
	for (int i = 0; i < connectMatrix.size(); i++){
		if (connectMatrix[i][pNum]!=0) return false;
	}
	return flag;
}


