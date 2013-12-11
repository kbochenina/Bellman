// WFSched.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ModelData.h"
#include <cstdlib>
#include "direct.h"
#include "Scheduler.h"
#include <string>
#include <iostream>

using namespace std;

enum SchedulingTypes { ONLY_BELLMAN, ONLY_GREEDY, MIXED };

int _tmain(int argc, wchar_t** argv)
{
	// fileSettings is a file with program settings
	// it is a first command line argument
	// if program is started without arguments, filename is "settings.txt"
	wstring fileSettings;
	if (argc == 1) {
		fileSettings=L"settings.txt";
	}
	else {
		fileSettings = argv[1];
	}
	string s(fileSettings.begin(),fileSettings.end());
	cout << "File settings name: " << s << endl;
	// set data
	DataInfo data(s);
	// init model data
	ModelData md(data);
	Scheduler sched(md);
	sched.SetSchedulingStrategy(ONLY_BELLMAN);	
	return 0;
}

