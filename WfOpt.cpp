// WfOpt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Model.h"
#include "UserException.h"
#include <cstdlib>
#include "direct.h"

	int T, delta, stages;
	ofstream ex;
	vector<int> stageBorders;
	bool canExecuteOnDiffResources;
	int outputFileNumber;
	bool directBellman;
	
void AddData(ofstream & r, int i){
	try{
		string resFileName = "staged_scheme_" + to_string((long long)i) + ".txt";
		ifstream res(resFileName);
		if (res.fail()) throw UserException("AddData(): unable to open res file");
		string s;
		while (!res.eof()){
			getline(res,s);
			r << s << endl;
		}
		res.close();
		if (remove(resFileName.c_str())!=0)
			throw UserException("AddData: unable to remove file");
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

int _tmain(int argc, wchar_t** argv)
{
	
	wstring fileResources, fileWorkflows, fileEx, fileSettings, fileXML;
	if (argc == 1) {
		fileSettings=L"settings.txt";
	}
	else {
		fileSettings = argv[1];
		wcout << fileSettings << endl;
	}

	try {
	
		double start = clock();
		string s(fileSettings.begin(),fileSettings.end());
		outputFileNumber = 0;
		Model m;
		m.InitSettings(s);
		_mkdir("result");
		_chdir("result");
		ofstream f("times.txt");
		ofstream r("stagedScheme_res.txt");
		double d = clock();
		m.SetData();
		f << "Time of setting data " <<  (clock() - d) / CLOCKS_PER_SEC  << " sec "<< endl;
		for (int i = 0; i < m.GetWorkflowNum(); i++){
			double stageTime = clock();
			m.StagedScheme(i);
			f << "Time of attempt " << i+1 << "  " << (clock() - stageTime)/ CLOCKS_PER_SEC << " sec "<< endl;
			AddData(r,i);
		}
		
		
		f << "Time of executing " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
		f << endl;
		f.close();
		r.close();
		ex.close();
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	return 0;
}



