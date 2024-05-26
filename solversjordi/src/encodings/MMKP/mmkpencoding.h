#ifndef MMKPENCODING_DEFINITION
#define MMKPENCODING_DEFINITION


#include <vector>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include "encoding.h"
#include "mmkp.h"
#include "solvingarguments.h"

using namespace std;

/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	OPTI_ENCODING,
	REDUCE_EO,
	OBJECTIVE,
	PRINT_ESSENCE,
	CREATE_LP
};


class MMKPEncoding : public Encoding {

private:

	MMKP * instance;
	Arguments<ProgramArg> * pargs;
	SolvingArguments * sargs;

	MDDBuilder * sharedmdd;

	vector<int> assignment;

public:
	MMKPEncoding(MMKP * instance, Arguments<ProgramArg>  * pargs, SolvingArguments * sargs);

	~MMKPEncoding();

	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);

	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);

	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);

	int getObjective() const;

	bool printSolution(ostream &os) const;

};

#endif

