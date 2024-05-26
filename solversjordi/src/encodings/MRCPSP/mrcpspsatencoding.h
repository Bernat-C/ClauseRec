#ifndef MRCPSPSATENCODING_DEFINITION
#define MRCPSPSATENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "mrcpspencoding.h"

using namespace std;

class MRCPSPSATEncoding : public MRCPSPEncoding {

private:
	AMOPBEncoding amopbenc;

public:
	MRCPSPSATEncoding(MRCPSP * instance, AMOPBEncoding amopbenc);
	SMTFormula *  encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	~MRCPSPSATEncoding();

};

#endif

