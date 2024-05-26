#include "rcpsptencoding.h"
#include <limits.h>
#include "util.h"


RCPSPTEncoding::RCPSPTEncoding(RCPSPT * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs) : Encoding() {
	this->ins = instance;
	this->sargs = sargs;
	this->pargs = pargs;
}

RCPSPTEncoding::~RCPSPTEncoding() {

}

int RCPSPTEncoding::getObjective() const{
	return starts.back();
}

bool RCPSPTEncoding::printSolution(ostream & os) const{
	ins->printSolution(os,starts);
	return true;
}
