#include <iostream>
#include "parser.h"
#include "errors.h"
#include "util.h"
#include "basiccontroller.h"
#include "basiceventhandler.h"
#include "mrcpspmaxencoding.h"
#include "rcpspmaxencoding.h"
#include "mrcpspmaxrelaxedencoding.h"
#include "solvingarguments.h"
#include "uboptimizer.h"
#include "buoptimizer.h"
#include "dicooptimizer.h"

using namespace std;
using namespace util;


enum ProgramArg {
	COMPUTE_UB,
	REDUCE_NR_DEMAND,
	RELAXED
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

	//Problem specific preprocessing
	arguments::bop("U","upper",COMPUTE_UB,true,
	"If 1, compute a better upper bound than the trivial one using a greedy heuristic. If an upper bound is specified with -u, upper is set to 0. Default: 1."),

	arguments::bop("R","reduce-nr",REDUCE_NR_DEMAND,true,
	"If 1, the non-renewable resource demands will be reduced by the minimum demand. Default: 1."),

	//Program behaviour parameters
	{arguments::bop("X","relaxed",RELAXED,true,
	"If 1 specified, print pb constraints to the specified file. Default: 1."),
	}

	},


	"Solve the Multi-mode Resource-Constrained Project Scheduling Problem with Minimum and Maximum Time Lags (MRCPSP/max)."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	//Parse the instance
	MRCPSPMAX * instance = parser::parseMRCPSPMAX(pargs->getArgument(0));

	instance->computeExtPrecs();

	if(pargs->getBoolOption(REDUCE_NR_DEMAND))
		instance->reduceNRDemandMin();

	int LB = sargs->getIntOption(LOWER_BOUND);
	int UB = sargs->getIntOption(UPPER_BOUND);
	if(LB==INT_MIN)
		LB = instance->trivialLB();
	if(UB==INT_MIN)
		UB = instance->trivialUB();


	//Compute tighter bounds by solving relaxed problems.
	//This process might certify optimality or unsatisfiability in some cases
	if(pargs->getBoolOption(RELAXED)){
		//Encode relaxed problem of finding a schedule s.t. the modes satisfy the precedences (and the non-renewable constraints?)
		MRCPSPMAXRelaxedEncoding encoding1(instance,sargs->getAMOPBEncoding());

		//Solve up to bottom relaxed problem
		DicoOptimizer opti1;
		Encoder * e = sargs->getEncoder(&encoding1);

		cout << "c ========== Minimizing relaxed problem ==========" << endl;

		int opt = opti1.minimize(e,LB,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));

		//If unsat, the original problem is unsat, end
		if(opt==INT_MIN)
			return 0;

		delete e;


		//If sat
		//Set max(LB,relaxed optimum) as LB
		LB=max(LB,opt);

		//Solve bottom up RCPSP/max with precedence-wise feasible modes
		vector<int> modes(instance->getNActivities()+2);
		encoding1.getModes(modes);
		RCPSPMAX * instance2 = instance->getRCPSPMAX(modes);

		instance2->computeExtPrecs();

		RCPSPMAXEncoding encoding2(instance2,sargs->getAMOPBEncoding());

		BUOptimizer opti2;
		e = sargs->getEncoder(&encoding2);

		cout << endl << endl;
		cout << "c ==== Minimizing RCPSP/max with fixed modes ====" << endl;

		opt = opti2.minimize(e,LB,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));


		delete e;

		//If optimum=LB, it is also optimum of MRCPSP/max, end
		if(opt==LB){
			cout << "c ==== Over-constrained optimum is valid optimum ===="<< endl;
			return 0;
		}

		//If we find a solution withing the initial UB, we can decrease it.
		//If we did not, it doesn't mean that the instance is unsatisfiable within the initial bounds, it only means that the instance is unsatisfiable within the given UB with the modes that we fixed (but with an unlimited UB the instance is satisfiable with this modes)
		if(opt != INT_MIN)
			UB = opt-1;
	}

	if(pargs->getBoolOption(RELAXED)){
		sargs->setOption(UPPER_BOUND,UB);
		sargs->setOption(LOWER_BOUND,LB);
	}

	//If proven sat but optimum not certified, optimize original problem with the obtained bounds
	MRCPSPMAXEncoding encoding(instance,sargs->getAMOPBEncoding());
	BasicController c(sargs,&encoding,true,LB,UB);
	c.run();

	return 0;
}

