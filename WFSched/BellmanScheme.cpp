#include "StdAfx.h"
#include "BellmanScheme.h"
#include <iostream>

void BellmanScheme::printInfo(){
	std::cout << "BellmanScheme is instantiated\n";
	std::cout << "koeff = " << eff->GetKoeff() << "\n";
}

double BellmanScheme::GetWFSchedule(Schedule &out){
	
	return 0.0;
}

BellmanScheme::~BellmanScheme(void)
{
}
