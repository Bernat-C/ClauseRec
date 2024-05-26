#include "prcpspencoding.h"
#include "assert.h"
#include <limits.h>


PRCPSPEncoding::PRCPSPEncoding(PRCPSP * instance) : Encoding() {
	this->ins = instance;
}

PRCPSPEncoding::~PRCPSPEncoding() {
}

int PRCPSPEncoding::getObjective() const{
	return starts.back();
}

bool PRCPSPEncoding::printSolution(ostream & os) const{
	ins->printSolution(os,starts,en_execucio);
	return true;
}

bool PRCPSPEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	
	//return false;
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->bvar("s",N+1,ub));
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				ef.f->addClause(!ef.f->bvar("x",i,t));
				
		return true;
	}
	else return false;
}