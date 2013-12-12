#include "StdAfx.h"
#include "Workflow.h"
#include "UserException.h"
#include <string>
#include <iostream>

double Workflow::GetExecTime( int pNum, int type, int cores) {
	try{
		if (pNum < 0 || pNum > packages.size()-1) 
			throw UserException("Workflow::GetExecTime() error. Wrong packageNum" + to_string(pNum));
		return packages[pNum].GetExecTime(type, cores);
	}
	catch (UserException& e){
		std::cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

Workflow::~Workflow(void)
{
}
