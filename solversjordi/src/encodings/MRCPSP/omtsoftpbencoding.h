#ifndef OMTSoftPBEncoding_DEFINITION
#define OMTSoftPBEncoding_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "mrcpspencoding.h"

using namespace std;

class OMTSoftPBEncoding : public MRCPSPEncoding {

private:

	bool omt;
	AMOPBEncoding amopbenc;

public:
	OMTSoftPBEncoding(MRCPSP * instance);
	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);

	~OMTSoftPBEncoding();

};

#endif

