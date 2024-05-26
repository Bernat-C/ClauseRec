#ifndef SMTTIMEENCODING_DEFINITION
#define SMTTIMEENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "mrcpspencoding.h"
#include "solvingarguments.h"

using namespace std;

class SMTTimeEncoding : public MRCPSPEncoding {

private:

	bool omt;
	SolvingArguments *sargs;


public:

	SMTTimeEncoding(MRCPSP * instance, SolvingArguments *sargs, bool omt);
	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	void assumeBounds(const EncodedFormula & ef, int LB, int ub, vector<literal> & assumptions);

	~SMTTimeEncoding();

};

#endif

