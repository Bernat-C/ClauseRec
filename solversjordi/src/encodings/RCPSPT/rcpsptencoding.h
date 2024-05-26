#ifndef RCPSPTENCODING_DEFINITION
#define RCPSPTENCODING_DEFINITION

#include <vector>
#include "rcpspt.h"
#include "smtformula.h"
#include "encoding.h"
#include "solvingarguments.h"


using namespace std;
using namespace smtapi;

/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	ENCODING,
	UPPER,
	COMPUTE_ENERGY_PREC
};

class RCPSPTEncoding : public Encoding {
private:

protected:
	vector<int> starts;
	RCPSPT * ins;
	SolvingArguments * sargs;
	Arguments<ProgramArg> * pargs;

public:
	RCPSPTEncoding(RCPSPT * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs);
	~RCPSPTEncoding();

	int getObjective() const;
	bool printSolution(ostream & os) const;

	virtual SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX) = 0;
};

#endif

