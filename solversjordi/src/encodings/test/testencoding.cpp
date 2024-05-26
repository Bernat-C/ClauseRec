#include "testencoding.h"
#include "util.h"
#include "smtapi.h"
#include "parser.h"
#include <cstdlib>

using namespace smtapi;

TestEncoding::TestEncoding(string filename) : Encoding()  {
}

SMTFormula * TestEncoding::encode(int lb, int ub){
	SMTFormula * f = new SMTFormula();

	int nvars = 8;
	int k = 4;

	//Skip already existing variables
	for(int i = 1; i <= nvars; i++){
		f->newBoolVar("x",i);
	}

	vector<literal> vars(nvars);
	vector<literal> varsnegated(nvars);
	vector<int> coefficients(nvars,1);
	for(int i=1; i <=nvars; i++){
		vars[i-1] = f->bvar("x",i);
		varsnegated[i-1] = !f->bvar("x",i);
	}

	//f->addAMK(vars,k,CARD_TOTALIZER);
	//f->addALK(vars,k,CARD_TOTALIZER);

	f->addPB(coefficients,vars,k,PB_MTO);
	f->addPB(coefficients,varsnegated,nvars-k-1,PB_MTO);

	return f;
}

void TestEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

}

bool TestEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	return false;
}

bool TestEncoding::printSolution(ostream & os) const{

	return false;
}

TestEncoding::~TestEncoding() {
}
