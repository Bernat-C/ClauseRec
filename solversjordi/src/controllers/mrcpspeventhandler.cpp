#include "mrcpspeventhandler.h"
#include "errors.h"

MRCPSPEventHandler::MRCPSPEventHandler(MRCPSPEncoding * encoding, SolvingArguments * sargs, MRCPSP * instance)
: BasicEventHandler(encoding,sargs){
	this->instance = instance;
}

MRCPSPEventHandler::~MRCPSPEventHandler(){

}

void MRCPSPEventHandler::afterSatisfiabilityCall(int lb, int ub, Encoder * encoder){
	if(sargs->getBoolOption(PRINT_CHECKS_STATISTICS)){
		cout << "c stats ";

		//Bounds and time
		cout << lb << ";";
		cout << ub << ";";
		cout << encoder->getCheckTime() << ";";

		//Formula sizes
		cout << encoder->getNBoolVars() << ";";
		cout << encoder->getNIntVars() << ";";
		cout << encoder->getNAtoms() << ";";
		cout << encoder->getNClauses() << ";";


		//Solving statistics
		cout << encoder->getNRestarts() << ";";
		cout << encoder->getNSimplify() << ";";
		cout << encoder->getNReduce() << ";";
		cout << encoder->getNDecisions() << ";";
		cout << encoder->getNPropagations() << ";";
		cout << encoder->getNConflicts() << ";";
		cout << encoder->getNTheoryPropagations() << ";";
		cout << encoder->getNTheoryConflicts() << ";";

		//Preprocessing statistis
		int total = instance->getNTWIncompatibilities() + instance->getNResourceDisjoints() + instance->getNPrecedenceIncompatibilities() + instance->getNResourceIncompatibilities();
		cout << instance->getNTWIncompatibilities() << ";";
		cout << instance->getNResourceDisjoints() << ";";
		cout << instance->getNPrecedenceIncompatibilities() << ";";
		cout << instance->getNResourceIncompatibilities() << ";";
		cout << total << ";";
		cout << instance->getNReducedNRDemands() << ";";
		cout << instance->getNResourceDisjoints() << endl;
	}
}







