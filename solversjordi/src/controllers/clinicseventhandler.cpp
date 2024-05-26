#include "clinicseventhandler.h"
#include "errors.h"

ClinicsEventHandler::ClinicsEventHandler(Clinics * instance, ClinicsEncoding * encoding, SolvingArguments * sargs, bool active) : BasicEventHandler(encoding, sargs){
	this->ins = instance;
	this->encoding = encoding;
	this->active = active;
}

ClinicsEventHandler::~ClinicsEventHandler(){

}

void ClinicsEventHandler::onSATSolutionFound(int & lb, int & ub, int & obj_val){
	if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS)){
		cout << "v ";
		if(!encoding->printSolution(cout))
			cout << " [Solution printing not implemented]";
		cout << endl;
	}

	if(active && ins->enforceActivity(encoding->getStarts())){
		cout << "c solution by activity" << endl;
		cout << "v ";
		if(!encoding->printSolution(cout))
			cout << " [Solution printing not implemented]";
		cout << endl;
		ub=encoding->getStarts()[ins->getNActivities()+1];
		obj_val=ub;
	}

}



