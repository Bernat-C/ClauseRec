#ifndef RCPSPTTIMEENCODING_DEFINITION
#define RCPSPTTIMEENCODING_DEFINITION

#include <vector>
#include "rcpspt.h"
#include "smtformula.h"
#include "rcpsptencoding.h"


using namespace std;
using namespace smtapi;

class RCPSPTTimeEncoding : public RCPSPTEncoding {
private:

protected:


public:

	RCPSPTTimeEncoding(RCPSPT * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs);
	~RCPSPTTimeEncoding();


	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	void assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions);
};

#endif

