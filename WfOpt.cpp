// WfOpt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Model.h"
#include <cstdlib>

	int T, delta, stages;
	ofstream ex;
	vector<int> stageBorders;
	
int _tmain(int argc, wchar_t** argv)
{
	
	wstring fileResources, fileWorkflows, fileEx, fileSettings, fileXML;
	if (argc == 1) {
		/*T = 43200;
		delta = 3600;
		fileResources = L"res_t4_p0.25_r4_c4_2";
		fileWorkflows = L"n-10_f-0.2_d-0.2_r-0.8_c-1_j-2.0";
		fileEx = L"defaultExperRes.txt";*/
		fileSettings=L"settings.txt";
	}
	else {
		/*T = 43200;
		delta = 1800;
		fileResources = argv[1];
		wcout << fileResources << endl;
		fileWorkflows = argv[2];
		wcout << fileWorkflows << endl;
		fileEx = argv[3];
		wcout << fileEx << endl;*/
		fileSettings = argv[1];
		wcout << fileSettings << endl;
	}

	try {
	/*	fileXML = fileResources + fileWorkflows;
		
		
		*/
		double start = clock();
		/* string sR(fileResources.begin(), fileResources.end()), 
		sW(fileWorkflows.begin(), fileWorkflows.end()), sS(fileSettings.begin(),fileSettings.end()), 
		sXML(fileXML.begin(), fileXML.end());*/
		string s(fileSettings.begin(),fileSettings.end());
		//ex << sW << endl;
		Model m;
		//m.Init(sR, sW, sS, sXML);
		m.InitSettings(s);
		m.StagedScheme(0);
		/*
		
		ex.close();*/
		cout << "Time of executing " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
		cout << endl;
		
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	return 0;
}

