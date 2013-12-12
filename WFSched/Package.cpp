#include "StdAfx.h"
#include "Package.h"
#include "UserException.h"
#include "string"
#include <iostream>


double Package::GetExecTime(int type, int cores){
	try{
		std::pair<int,int> typeCore = make_pair(type,cores);
		auto it = execTimes.find(typeCore);
		if (it==execTimes.end()) 
			throw UserException("Package::GetExecTime() : combination of type " + to_string(type) + 
			"  and cores " + to_string(cores) + " not found");
		return execTimes[std::make_pair(type,cores)];
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}


Package::~Package(void)
{
}
