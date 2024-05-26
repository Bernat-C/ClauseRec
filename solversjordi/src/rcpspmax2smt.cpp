#include <iostream>
#include "parser.h"
#include "errors.h"
#include "util.h"
#include "basiccontroller.h"
#include "rcpspmaxencoding.h"
#include "solvingarguments.h"

using namespace std;
using namespace util;


enum ProgramArg {
};



int main(int argc, char **argv) {
	Arguments<ProgramArg> * pargs
	= new Arguments<ProgramArg>(

	//Program arguments
	{
	arguments::arg("filename","Instance file name.")
	},
	1,


	//==========Program options===========
	{

	},


	"Solve the Resource-Constrained Project Scheduling Problem with Minimum and Maximum Time Lags (RCPSP/max)."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	//Parse the instance
	RCPSPMAX * instance = parser::parseRCPSPMAX(pargs->getArgument(0));

	instance->computeExtPrecs();

	RCPSPMAXEncoding encoding(instance,sargs->getAMOPBEncoding());
	BasicController c(sargs,&encoding,true,instance->trivialLB(),instance->trivialUB());
	c.run();

	return 0;
}

