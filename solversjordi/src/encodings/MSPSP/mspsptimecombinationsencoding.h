#ifndef MSPSPTIMECOMBINATIONSENCODING_DEFINITION
#define MSPSPTIMECOMBINATIONSENCODING_DEFINITION

#include <vector>
#include "mspsp.h"
#include "smtformula.h"
#include "mspspencoding.h"
#include "solvingarguments.h"
#include "arguments.h"


using namespace std;
using namespace smtapi;

class MSPSPTimeCombinationsEncoding : public MSPSPEncoding {
private:

protected:

  Arguments<ProgramArg> * pargs;

  SolvingArguments * sargs;

public:

	MSPSPTimeCombinationsEncoding(MSPSP * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs);
	~MSPSPTimeCombinationsEncoding();


	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	void assumeBounds(const EncodedFormula & ef, int LB, int ub, vector<literal> & assumptions);


};

#endif

