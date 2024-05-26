#include "parser.h"
#include "errors.h"
#include "basiccontroller.h"
#include "mspspencoding.h"
#include "mspsptimeencoding.h"
#include "mspspnbddtimeencoding.h"
#include "mspsptimecombinationsencoding.h"
#include "mspsptimecombinationsencoding2.h"
#include "mspsptaskcombinationsencoding2.h"
#include "solvingarguments.h"
#include "cpoptmspspencoder.h"


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

	arguments::bop("","implied1",IMPLIED1,false,
	"Add the implied constrant: the number of activities requiring skill 'l' at a particular time is not greater than the number of resources that master 'l'. Default: 0."),

	arguments::bop("","implied2",IMPLIED2,false,
	"Add the implied constrant: the number of activities running at a particular time is not greater than the number of resources. Default: 0."),

	arguments::bop("","implied3",IMPLIED3,true,
	"Add the implied constrant: for any combination of skills (dominance filtering applies), the number of activities running at a particular time is not greater than the number of resources that master those skills. Default: 1."),

	arguments::bop("","symbreak",SYMBREAK,false,
	"Add a symetry breaking constrant which set a preference order on the use of identical resources. Default: 0."),

	arguments::bop("U","upper",UPPER,true,
	"Try to compute a first UB with a greedy heuristic. Default: 1."),

    arguments::bop("","cpopt",CPOPT,false,
                   "Use CP Optimizer to solve. Default: 0."),

	arguments::bop("O","omt",OMT,false,
	"Encode an OMT problem. Default: 0."),

	arguments::bop("","unit",UNIT,false,
	"Make all durations unit. Default: 0."),

	arguments::bop("","gerarquic",GERARQUIC,false,
	"Make the masterys gerarquic. Default: 0."),

	arguments::sop("E","encoding",ENCODING,"time",{"time","timegerarquic","timecomb","timecomb2","taskcomb2","nbddtime"},
	"Encoding to use. Default: time."),

	arguments::bop("I","intersections",INTERSECTIONS,false,
	"Channel intersections in decomposition of resource usage variables . Default: 0."),

	arguments::bop("","energy",ENERGY,true,
	"Compute energetic precedences. Default: 1."),

	arguments::bop("F","full",FULL,true,
	"If true requires that the number of suplied skills is exactly the demand. Else, it must be at least the demand (still correct solutions in output). Default: 1."),

	arguments::bop("","print",PRINT,false,
	"If true, prints the instance (after possible modifications) and exits. Default: 0."),

	arguments::bop("","printdzn",PRINTDZN,false,
	"If true, prints the pre-pocessed information in dzn format and exits. Default: 0."),

	arguments::bop("","print-bounds",PRINTBOUNDS,false,
	"If true, prints the instance (after possible modifications). Default: 0."),

	arguments::bop("","lp-ct-real",LP_CT_REAL,false,
	"If true, outputs countinuous time MILP model with continuous start time variables. Default: 0."),

	arguments::bop("","lp-ct-int",LP_CT_INT,false,
	"If true, outputs countinuous time MILP model with integer start time variables. Default: 0."),

	arguments::bop("","lp-dt",LP_DT,false,
	"If true, outputs discrete time MILP model. Default: 0."),

	arguments::bop("","lp-ddt",LP_DDT,false,
	"If true, outputs discrete time DDT MILP model. Default: 0."),

	arguments::bop("","lp-jc-int",LP_JC_INT,false,
	"If true, outputs JC MILP model with integer variables. Default: 0."),

	arguments::bop("","lp-jc-real",LP_JC_REAL,false,
	"If true, outputs JC MILP model wit real variables. Default: 0."),

	arguments::bop("","lp-hr",LP_HR,false,
	"If true, half reification in LP models. Default: 0."),

	arguments::bop("J","json",JSON,false,
	"If true, outputs the instance in JSON format and exits. Default: 0.")

	},

	"Solve the Multi-Skill Project Scheduling Problem (MSPSP)."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MSPSP * instance = parser::parseMSPSP(pargs->getArgument(0));
	instance->preProcess();

	int initlb=instance->trivialLB();

	if(pargs->getBoolOption(ENERGY)){
		instance->computeEnergyPrecedences();
		instance->recomputeExtPrecs();
	}

	if(pargs->getStringOption(ENCODING)=="nbddtime")
		instance->preProcessNBDD();

	/*ofstream os;
	os.open("figura.dot");
	instance->getNBDD(1)->createGraphviz(instance,instance->getNBDDOrderedResources(4),instance->getDemands(4),os);
	os.close();
    exit(0);*/
	/*for(int i = 0; i < instance->getNActivities()+2; i++){
		ofstream os((to_string(i)+"_exemple.dot").c_str());
		if(!os.is_open()){
			cerr << "Error: could not open file" << endl;
			exit(BADFILE_ERROR);
		}
		instance->getNBDD(i)->createGraphviz(instance,instance->getNBDDOrderedResources(i),instance->getDemands(i),os);
		os.close();
	}

	*/

	if(pargs->getBoolOption(UNIT))
		instance->makeDurationsUnitary();

	if(pargs->getBoolOption(GERARQUIC))
		instance->makeResourceMasteryGerarquic();

	if(pargs->getBoolOption(PRINT)){
		cout << *instance << endl;
		exit(0);
	}



	bool sat=true;
	int UB = sargs->getIntOption(UPPER_BOUND);
	int LB = sargs->getIntOption(LOWER_BOUND);
	if(LB==INT_MIN)
		LB=instance->trivialLB();
	if(pargs->getBoolOption(UPPER) && sargs->getIntOption(UPPER_BOUND)== INT_MIN){
		vector<int> starts;
		vector<vector<pair<int,int> > > assignment;
		UB = instance->computePSS(starts,assignment);
		if(UB >= 0){
		 	if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS)){
				cout << "c Solution found by greedy heuristic:" << endl;
				if(sargs->getBoolOption(PRINT_CHECKS))
					BasicController::onNewBoundsProved(LB,UB);
				cout << "v ";
				instance->printSolution(cout, starts,assignment);
				cout << endl << endl;;
			}
		}
		else{
			if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
				cout << "c No solution found by greedy heuristic" << endl;
		}
	}
    if(UB < 0){
		UB = instance->trivialUB();
		sat=false;
    }

    if(pargs->getBoolOption(PRINTDZN)){
		instance->printDZN(cout, UB);
		exit(0);
	}

    if(pargs->getBoolOption(PRINTBOUNDS)){
		cout << "c lb lbsym ub " << initlb << " " << LB << " " << UB << endl;
		exit(0);
	}

    if(pargs->getBoolOption(CPOPT)){
#ifdef CPOPTIMIZER
        CPOPTMSPSPEncoder enc(pargs);
        enc.solve(instance,UB);
        exit(0);
#else
        cerr << "Error: compiled without support for CP Optimizer" << endl;
        exit(0);
#endif
    }

    if(pargs->getBoolOption(LP_CT_REAL)){
    	instance->printCTLPInstance(cout,UB,false);
    	exit(0);
    }
    if(pargs->getBoolOption(LP_CT_INT)){
    	instance->printCTLPInstance(cout,UB,true);
    	exit(0);
    }
    if(pargs->getBoolOption(LP_DT)){
    	instance->printDTLPInstance(cout,UB,true);
    	exit(0);
    }
    if(pargs->getBoolOption(LP_DDT)){
    	instance->printDTLPInstance(cout,UB,false);
    	exit(0);
    }
    if(pargs->getBoolOption(LP_JC_INT)){
    	instance->printJCLPInstance(cout, UB, true, true, pargs->getBoolOption(LP_HR));
    	exit(0);
    }
    if(pargs->getBoolOption(LP_JC_REAL)){
    	instance->printJCLPInstance(cout, UB, true, false, pargs->getBoolOption(LP_HR));
    	exit(0);
    }
    
	MSPSPEncoding * encoding;
	if(pargs->getStringOption(ENCODING)=="time")
		encoding = new MSPSPTimeEncoding(instance,sargs,pargs);
	else if(pargs->getStringOption(ENCODING)=="timegerarquic")
		encoding = new MSPSPTimeEncoding(instance,sargs,pargs);
	else if(pargs->getStringOption(ENCODING)=="timecomb")
		encoding = new MSPSPTimeCombinationsEncoding(instance,sargs,pargs);
	else if(pargs->getStringOption(ENCODING)=="timecomb2")
		encoding = new MSPSPTimeCombinationsEncoding2(instance,sargs,pargs);
	else if(pargs->getStringOption(ENCODING)=="taskcomb2")
		encoding = new MSPSPTaskCombinationsEncoding2(instance,sargs,pargs);
	else //nbddtime
		encoding = new MSPSPNBDDTimeEncoding(instance,sargs,pargs);


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

