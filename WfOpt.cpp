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
	
	wstring fileResources, fileWorkflows, fileEx, fileSettings;
	if (argc == 1) {
		T = 43200;
		delta = 3600;
		fileResources = L"res_t4_p0.25_r4_c4_2";
		fileWorkflows = L"n-5_f-0.2_d-0.8_r-0.8_c-0_j-4.1";
		fileEx = L"defaultExperRes.txt";
		fileSettings=L"settings.txt";
	}
	else {
		T = 43200;
		delta = 3600;
		fileResources = argv[1];
		wcout << fileResources << endl;
		fileWorkflows = argv[2];
		wcout << fileWorkflows << endl;
		fileEx = argv[3];
		wcout << fileEx << endl;
		fileSettings = argv[4];
		wcout << fileSettings << endl;
	}

	try {
		string openErr = "File with experiment result cannot be open";
		ex.open(fileEx, ios::app);
		if (ex.fail()) throw openErr;
		
		stages = T/delta;
		for (int i = 0; i <= T; i+=delta) stageBorders.push_back(i);
		double start = clock();
		 string sR(fileResources.begin(), fileResources.end()), 
			 sW(fileWorkflows.begin(), fileWorkflows.end()), sS(fileSettings.begin(),fileSettings.end());
		ex << sW << endl;
		Model m;
		m.Init(sR, sW, sS);
		for (int i = stages-1; i >= 0; i--)
			m.GetStageInformation(i);
		m.DirectBellman();
		//m.SetFullState();
		//ex << "Time of set full states " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
		//ex << m.GetStatesCount() << endl;
		//for (int i = stages; i > 0; i--)
		//	m.GetStageInformation(i);
	
		//m.DirectBellman();
		//cout << "Max efficiency: " << m.GetMaxEfficiency() << endl;
	
		ex.close();
		cout << "Time of executing " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
		cout << endl;
		system("pause");
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	return 0;
}

