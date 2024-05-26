#include <iostream>
#include <fstream>
#include "assert.h"
#include "parser.h"
#include "errors.h"
#include "encoder.h"
#include "solvingarguments.h"
#include "basiccontroller.h"
#include "mrcpspeventhandler.h"
#include "mrcpsp.h"
#include "mrcpspencoding.h"
#include "smttimeencoding.h"
#include "smttaskencoding.h"
#include "rcpspencoding.h"
#include "order.h"
#include "doubleorder.h"
#include "omtsatpbencoding.h"
#include "omtsoftpbencoding.h"
#include "mrcpspsatencoding.h"


using namespace std;
using namespace util;

/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	COMPUTE_UB,
	ACTIVITY,
	REDUCE_NR_DEMAND, 
	COMPUTE_TW_INCOMP,
	COMPUTE_DISJOINTS,
	COMPUTE_ENERGY_PREC,

	PRCPSPDUMMY,

	ENCODING,

    ORDER
};




bool getSatModes(SolvingArguments * sargs, vector<int> &modes, MRCPSP * instance){
	bool sat;
	float time;
	MRCPSPSATEncoding * enc = new MRCPSPSATEncoding(instance, sargs->getAMOPBEncoding());

	Encoder * e = sargs->getEncoder(enc);
	e->setProduceModels(true);

	sat = e->checkSAT(0,0);

	if(sat)
		enc->getModes(modes);

	delete enc;
	delete e;

	return sat;
}



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

	//Problem specific preprocessing
	arguments::bop("U","upper",COMPUTE_UB,true,
	"If 1, compute a better upper bound than the trivial one using a greedy heuristic. If an upper bound is specified with -u, upper is set to 0. Default: 1."),

	arguments::bop("A","active",ACTIVITY,true,
	"If 1, compute an active solution for scheduling problems each time a new solution is found. Default: 1."),

	arguments::bop("R","reduce-nr",REDUCE_NR_DEMAND,true,
	"If 1, the non-renewable resource demands will be reduced by the minimum demand. Default: 1."),

	arguments::bop("","tw-incomp",COMPUTE_TW_INCOMP,false,
	"If 1, compute the time window incompatibilities. Default: 0."),

	arguments::bop("","disjoint-incomp",COMPUTE_DISJOINTS,false,
	"If 1, compute the activity disjoint use of resources notion of incompatibility. Default: 0."),

	arguments::bop("","energy-prec",COMPUTE_ENERGY_PREC,true,
	"If 1, compute energy based predecendes. Default: 1."),


	arguments::bop("","prcpsp-dummy",PRCPSPDUMMY,false,
	"If 1, splits each activity by a number of pieces equal to its duration transforming the problem to a dummy PRCPSP. Default: 0."),


	//Encoding parameters
	arguments::sop("E","encoding",ENCODING,"smttime",
	{"smttime","smttask","omtsatpb","omtsoftpb","order","doubleorder","satorder"},
	"Encoding of the problem. SMT-based require an SMT solver. Default: smttime."),


        
    arguments::iop("","use-order",ORDER,0,
            "If 1, decide only on the problem variables, using VSIDS. If 2, use a predefined order of the problem variables for the decisions. Only availabe for Minisat. Default: 0."),
        
	},
	"Solve the Multi-mode Resource-Constrained Project Scheduling Problem (MRCPSP). Default: 0."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);
	
	MRCPSP * instance;
	MRCPSP * instance2;

	std::string filename = pargs->getArgument(0);
	sargs->setOption(FILE_PREFIX,filename);

	if(!pargs->getBoolOption(PRCPSPDUMMY)) {
		instance = parser::parseMRCPSP(pargs->getArgument(0));
	}
	else { // Si volem codificar a PRCPSP des de prcpsp
		instance = parser::parseMRCPSPasPRCPSPfromRCP(pargs->getArgument(0));
		instance2 = parser::parseMRCPSP(pargs->getArgument(0));
	}

	MRCPSPEncoding * encoding = NULL;
	bool opti = sargs->getStringOption(OPTIMIZER)=="native";
	string s_encoding = pargs->getStringOption(ENCODING);

	if(s_encoding=="smttime")
		encoding = new SMTTimeEncoding(instance,sargs,opti);
	else if(s_encoding=="smttask")
		encoding = new SMTTaskEncoding(instance,sargs,opti);
	else if(s_encoding=="order")
		encoding = new Order(instance,sargs->getAMOPBEncoding(),pargs->getIntOption(ORDER),opti);
	else if(s_encoding=="doubleorder")
		encoding = new DoubleOrder(instance, sargs->getAMOPBEncoding(), opti);
	else if(s_encoding=="omtsatpb")
		encoding = new OMTSATPBEncoding(instance);
	else if(s_encoding=="omtsoftpb")
		encoding = new OMTSoftPBEncoding(instance);
	else if(s_encoding=="satorder")
		encoding = new RCPSPEncoding(instance,sargs->getAMOPBEncoding());

	instance->computeExtPrecs();
	instance->computeSteps();


	if(pargs->getBoolOption(COMPUTE_ENERGY_PREC)){
		instance->computeEnergyPrecedences();
		instance->recomputeExtPrecs();
	}

	if(pargs->getBoolOption(REDUCE_NR_DEMAND)){
		instance->reduceNRDemandMin();
	}

	int UB = sargs->getIntOption(UPPER_BOUND);
	int LB = sargs->getIntOption(LOWER_BOUND);
	if(LB==INT_MIN)
		LB=instance->trivialLB();
		
	vector<int> starts;
	vector<int> modes;

	if(pargs->getBoolOption(COMPUTE_UB) && sargs->getIntOption(UPPER_BOUND)== INT_MIN){

		bool sat = getSatModes(sargs,modes,instance);
		if(sat){
			UB = instance->computePSS(starts,modes);
			if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS)){
				cout << "c Solution found by greedy heuristic:" << endl;
				if(sargs->getBoolOption(PRINT_CHECKS))
					BasicController::onNewBoundsProved(LB,UB);
				cout << "v ";
				if(!pargs->getBoolOption(PRCPSPDUMMY)) {
					instance->printSolution(cout, starts, modes);
				}
				else {
					instance2->printPRCPSPSolution(cout, starts, modes);
				}
				cout << endl << endl;
			}
			//If just satisfiability check, and satisfiability detected in preprocess, print SAT and exit
			if(sargs->getStringOption(OPTIMIZER)=="check"){
				BasicController::onProvedSAT();
				return 0;
			}
		}
		else{
			BasicController::onProvedUNSAT();
			return 0;
		}
	}
	
	if(sargs->getBoolOption(OUTPUT_ENCODING)){
		FileEncoder * e = sargs->getFileEncoder(encoding);
		SMTFormula * f = encoding->encode(LB,UB);
		e->createFile(std::cout,f);
		delete e;
		delete f;
	}
	else{

		if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
			cout << "c Trying to improve solution with exact solving:" << endl;

		Optimizer * opti = sargs->getOptimizer();
		Encoder * e = sargs->getEncoder(encoding);
		

		if(sargs->getBoolOption(PRINT_CHECKS_STATISTICS)){
			opti->setAfterSatisfiabilityCall([=](int lb, int ub, Encoder * encoder){BasicController::afterSatisfiabilityCall(lb,ub,encoder);});
			opti->setAfterNativeOptimizationCall([=](int lb, int ub, Encoder * encoder){BasicController::afterNativeOptimizationCall(lb,ub,encoder);});
		}

		if(sargs->getBoolOption(PRINT_CHECKS))
			opti->setOnNewBoundsProved([=](int lb, int ub){BasicController::onNewBoundsProved(lb,ub);});
		
		if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
			opti->setOnSATSolutionFound([=](int & lb, int & ub, int & obj_val){BasicController::onSATSolutionFound(lb,ub,obj_val,encoding);});
		
		UB--; //Solution for UB already found, start with next value
		
		int opt = opti->minimize(e,LB,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));
		//std::cout << "..." << opt << std::endl;
		if(opt==INT_MIN) { //If no better solution found than the one found in the greedy heuristic, that is the objective
			opt = UB+1;
			if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_OPTIMAL_SOLUTION)) {
				if (!pargs->getBoolOption(PRCPSPDUMMY)) 
					instance->printSolution(std::cout, starts, modes);
				else {
					instance2->printPRCPSPSolution(cout, starts, modes);
				}
			}
			std::cout << std::endl;
			BasicController::onProvedOptimum(opt);
		}
		else
			if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_OPTIMAL_SOLUTION))
				if(pargs->getBoolOption(PRCPSPDUMMY)) {
					encoding->getStartsAndModes(starts, modes);
					instance2->printPRCPSPSolution(std::cout, starts, modes);
					BasicController::onProvedOptimum(opt);
				}
				else
					BasicController::onProvedOptimum(opt,encoding);

			else
				BasicController::onProvedOptimum(opt);

		delete opti;
		delete e;

	}

	delete instance;
	delete pargs;
	delete sargs;
	delete encoding;

	return 0;
}

