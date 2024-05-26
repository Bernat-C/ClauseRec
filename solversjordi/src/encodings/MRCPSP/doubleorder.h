#ifndef DOUBLEORDER_DEFINITION
#define DOUBLEORDER_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include "mrcpspencoding.h"
#include <stdlib.h>

using namespace std;

class DoubleOrder : public MRCPSPEncoding {

private:
	bool maxsat;
	AMOPBEncoding amopbenc;

public:

	DoubleOrder(MRCPSP * instance,  AMOPBEncoding amopbenc, bool maxsat = false);
	~DoubleOrder();

	SMTFormula * encode(int vMin = INT_MIN, int vMax = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
};

#endif

