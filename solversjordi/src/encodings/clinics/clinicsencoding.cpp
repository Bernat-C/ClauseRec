#include "clinicsencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>
#include <stdlib.h>

ClinicsEncoding::ClinicsEncoding(Clinics * instance) : Encoding()  {

	this->ins = instance;
	this->filename = "";
}

ClinicsEncoding::~ClinicsEncoding() {

}

void ClinicsEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

	this->starts.resize(ins->getNActivities()+2);
	for (int i=0; i<ins->getNActivities()+2; i++)
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
}

int ClinicsEncoding::getObjective() const{
	return starts[ins->getNActivities()+1];
}

void ClinicsEncoding::setPrintCharts(const string & filename){
	this->filename = filename;
}

bool ClinicsEncoding::printSolution(ostream & os) const {
	ins->printSolution(os,starts);
	ins->makeChart(filename,starts);
	return true;
}

vector<int> & ClinicsEncoding::getStarts(){
	return starts;
}
