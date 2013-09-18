// WfOpt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Node.h"
#include "Package.h"
#include "Model.h"

int _tmain(int argc, _TCHAR* argv[])
{
	int T, delta;
	string fileResources, fileWorkflows;
	if (argc == 1) {
		T = 43200;
		delta = 5400;
		fileResources = "res_t2_p0.5_r2_c2_2";
		fileWorkflows = "n-10_f-0.2_d-0.2_r-0.8_c-1_j-2.1";
	}

	double start = clock();
	 
	Model m (T, delta);
	m.Init(fileResources, fileWorkflows);
	

	m.SetFullState();
	cout << "Time of set full states " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
	//m.GetStageInformation(8);
	//m.Simple();
	int stages = T/delta-1;
	for (int i = stages; i > 0; i--)
		m.GetStageInformation(i);
	/*m.GetStageInformation(3);
	m.GetStageInformation(2);
	m.GetStageInformation(1);*/
	m.DirectBellman();
	//cout << "Max efficiency: " << m.GetMaxEfficiency() << endl;
	//m.Zhadina();
	//m.GetFullState("fullState.txt");
	//m.OneStep(3);
	//m.OneStep(2);
	//m.CreateTuples();
	//m.SetPackagesAvaliableStates();
	//m.SetStates(20000);
	//m.ShowPackageAvaliableState(0);
	//m.ShowTuples();
	//m.SetFreeCores(20000);
	//m.GetOneStageControls(20000);
	//ofstream file("709.txt");
	//m.GetStateControls(709, file);
	cout << "Time of executing " <<  (clock() - start) / CLOCKS_PER_SEC  << " sec "<< endl;
	cout << endl;
	system("pause");
	return 0;
}

