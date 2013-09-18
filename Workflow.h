#pragma once
class Workflow
{
	vector <int> packageNumbers; 
	map <int, vector<int>> links; // (1, (2,3)) means package 1 execution required for package 2 and package 3 execution
	map <int, vector<int>> directlinks; 
public:
	Workflow(vector <int> &p, map <int, vector<int>> &l, map <int, vector<int>> &dl);
	int Size(){return packageNumbers.size();}
	vector <int>& GetPackageNumbers() {return packageNumbers;}
	bool IsInit(int packageNumber){
		map <int, vector<int>>::iterator it = links.begin();
		for (it;it!=links.end(); it++){
			vector <int>::iterator i = find((*it).second.begin(),(*it).second.end(),packageNumber);
			if (i!= (*it).second.end()) return false;
		}
		return true;
		
	}
	bool isPackageIn(int packageNumber)
	{
		vector <int>::iterator it; it = find (packageNumbers.begin(), packageNumbers.end(), packageNumber);
		if (it!=packageNumbers.end()) return true;
		return false;
	}
	bool isPackagesBothIn(int num1, int num2){
		vector <int>::iterator it1, it2; 
		it1 = find (packageNumbers.begin(), packageNumbers.end(), num1);
		it2 = find (packageNumbers.begin(), packageNumbers.end(), num2);
		if (it1!=packageNumbers.end() && it2!=packageNumbers.end()) return true;
		else return false;
	}
	bool isDepends(int first, int second)
	{
		map <int, vector<int>>::iterator it = links.find(first);
		if (it!=links.end()){
		vector <int>::iterator vi = find((*it).second.begin(),(*it).second.end(),second);
		if (vi!= (*it).second.end()) return true;
		}
		return false;
	}

	bool isDirectDepends(int first, int second)
	{
		map <int, vector<int>>::iterator it = directlinks.find(first);
		vector <int>::iterator vi = find((*it).second.begin(),(*it).second.end(),second);
		if (vi!= (*it).second.end()) return true;
		return false;
	}

	vector <int> GetDependency(int packageNumber){
		vector <int> result;
		map <int, vector<int>>::iterator il = links.begin();
		for (; il!=links.end(); il++){
			vector <int>::iterator it = (*il).second.begin();
			for (;it!=(*il).second.end(); it++){
				if (*it == packageNumber) result.push_back((*il).first);
			}

		}
		return result;
	}
	~Workflow(void);
};

