#include "stdafx.h"
#include "SchedulingFactory.h"
#include "UserException.h"
#include <iostream>

enum Algorithms{ BELLMAN = 1, GREEDY = 2 };

SchedulingFactory::SchedulingFactory(void)
{
}


SchedulingFactory::~SchedulingFactory(void)
{
}

unique_ptr<SchedulingMethod> SchedulingFactory::GetMethod(DataInfo &d, int uid, int wfNum){
	try{
		switch (uid){
		case BELLMAN:
			return unique_ptr<SchedulingMethod> (new BellmanScheme(d, uid, wfNum));
		default:
			throw UserException("SchedulingFactory::GetMethod() error. No valid algorithm found. Current algorithm uid = " + to_string(uid));
		}
	}
	catch (UserException& e){
		cout << "error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}
