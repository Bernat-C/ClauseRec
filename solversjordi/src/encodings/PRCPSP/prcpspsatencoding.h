#ifndef PRCPSPSATENCODING_DEFINITION
#define PRCPSPSATENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "prcpspencoding.h"

using namespace std;

class PRCPSPSATEncoding : public PRCPSPEncoding {

private:
	AMOPBEncoding amopbenc;

public:
	PRCPSPSATEncoding(PRCPSP * instance, AMOPBEncoding amopbenc);
	SMTFormula *  encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	~PRCPSPSATEncoding();

};

#endif

