#include "parser.h"
#include "errors.h"
#include "basiccontroller.h"
#include "rcpspttimeencoding.h"
#include "rcpspttaskencoding.h"
#include "solvingarguments.h"

using namespace std;
using namespace util;



int main(int argc, char **argv) {

	Arguments<ProgramArg> * pargs
	= new Arguments<ProgramArg>(

	//Program arguments
	{arguments::arg("filename","Instance file name.")},
	1,

	//Program options

	{
	arguments::bop("U","upper",UPPER,true,
		"Try to compute a first UB with a greedy heuristic. Default: 1."),

	arguments::sop("E","encoding",ENCODING,"time",{"time","task"},
		"Encoding. Default: time."),

	arguments::bop("","energy-prec",COMPUTE_ENERGY_PREC,true,
	"If 1, compute energy based predecendes. Default: 1.")

	},




	"Solve the Resource-Constrained Project Scheduling Problem with Time-Dependent Recource Capacities and Requests (RCPSP/t)."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	RCPSPT * instance = parser::parseRCPSPT(pargs->getArgument(0));

	instance->generateParam();
	exit(0);

	instance->computeExtPrecs();
	instance->computeSteps();

	if(pargs->getBoolOption(COMPUTE_ENERGY_PREC)){
		instance->computeEnergyPrecedences();
		instance->recomputeExtPrecs();
	}


	RCPSPTEncoding * encoding = NULL;
	if(pargs->getStringOption(ENCODING)=="time")
		encoding = new RCPSPTTimeEncoding(instance, sargs, pargs);
	else //pargs->getStringOption(ENCODING)=="task"
		encoding = new RCPSPTTaskEncoding(instance, sargs, pargs);

	bool sat=true;
	int UB = sargs->getIntOption(UPPER_BOUND);
	int LB = sargs->getIntOption(LOWER_BOUND);
	if(LB==INT_MIN)
		LB=instance->trivialLB();
	if(pargs->getBoolOption(UPPER) && sargs->getIntOption(UPPER_BOUND)== INT_MIN){
		vector<int> starts;
		UB = instance->computePSS(starts);
		if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS)){
			if(UB >= 0){
				cout << "c Solution found by greedy heuristic:" << endl;
				if(sargs->getBoolOption(PRINT_CHECKS))
					BasicController::onNewBoundsProved(LB,UB);
				cout << "v ";
				instance->printSolution(cout, starts);
				cout << endl << endl;
			}
			else{
				cout << "c No solution found by greedy heuristic" << endl;
			}
		}

	}
    if(UB < 0){
		UB = instance->trivialUB();
		sat=false;
    }
	

	if(sargs->getBoolOption(OUTPUT_ENCODING)){
		FileEncoder * e = sargs->getFileEncoder(encoding);
		SMTFormula * f = encoding->encode(LB,UB);
		e->createFile(std::cout,f);
		delete e;
		delete f;
	}
	else{
		//If just satisfiability check, and satisfiability detected in preprocess, print SAT and exit
		if(sargs->getStringOption(OPTIMIZER)=="check" && sat){
			BasicController::onProvedSAT();
			return 0;
		}

		if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
			cout << "c Trying to improve solution with exact solving:" << endl;
		
		Optimizer * opti = sargs->getOptimizer();
		Encoder * e = sargs->getEncoder(encoding);
		int opt;

		if(sargs->getBoolOption(PRINT_CHECKS_STATISTICS)){
			opti->setAfterSatisfiabilityCall([=](int lb, int ub, Encoder * encoder){BasicController::afterSatisfiabilityCall(lb,ub,encoder);});
			opti->setAfterNativeOptimizationCall([=](int lb, int ub, Encoder * encoder){BasicController::afterNativeOptimizationCall(lb,ub,encoder);});
		}

		if(sargs->getBoolOption(PRINT_CHECKS))
			opti->setOnNewBoundsProved([=](int lb, int ub){BasicController::onNewBoundsProved(lb,ub);});
		
		if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
			opti->setOnSATSolutionFound([=](int & lb, int & ub, int & obj_val){BasicController::onSATSolutionFound(lb,ub,obj_val,encoding);});
		
		if(sat)
			UB--; //Solution for UB already found, start with next value
		
		opt = opti->minimize(e,LB,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));

		if(sat && opt==INT_MIN) //If no better solution found than the one found in the greedy heuristic, that is the objective
			opt = UB+1;
		else if(!sat && opt!=INT_MIN) //Solution not found in greedy heuristic but found in exact solving
			sat=true;

		if(sat){
			if(sargs->getStringOption(OPTIMIZER)=="check")
				BasicController::onProvedSAT();
			else
				BasicController::onProvedOptimum(opt);
		}
		else
			BasicController::onProvedUNSAT();

		delete opti;
		delete e;

	}

	return 0;
}

