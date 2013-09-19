// WfOpt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Model.h"
#include <cstdlib>

	int T, delta, stages;
	vector<int> stageBorders;
	
int _tmain(int argc, wchar_t** argv)
{
	wstring fileResources, fileWorkflows, fileEx;
	if (argc == 1) {
		T = 43200;
		delta = 3600;
		fileResources = L"res_t2_p0.5_r2_c2_2";
		fileWorkflows = L"n-20_f-0.1_d-0.2_r-0.2_c-0_j-2.0";
		fileEx = L"defaultExperRes.txt";
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
	}

	try {
		string openErr = "File with experiment result cannot be open";
		ofstream ex(fileEx, ios::app);
		if (ex.fail()) throw openErr;
		
		stages = T/delta;
		for (int i = 0; i <= T; i+=delta) stageBorders.push_back(i);
		double start = clock();
		 string sR(fileResources.begin(), fileResources.end()), 
			 sW(fileWorkflows.begin(), fileWorkflows.end());
		ex << sW << endl;
		Model m;
		m.Init(sR, sW);
		

		m.SetFullState();
		ex << "Time of set full states " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
		ex << m.GetStatesCount() << endl;
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

