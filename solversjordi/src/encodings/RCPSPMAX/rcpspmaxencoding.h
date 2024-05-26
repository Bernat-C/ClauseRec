#ifndef RCPSPMAXENCODING_DEFINITION
#define RCPSPMAXENCODING_DEFINITION

#include <vector>
#include "mrcpspmax.h"
#include "smtformula.h"
#include "encoding.h"


using namespace std;
using namespace smtapi;

class RCPSPMAXEncoding : public Encoding {
private:

protected:

  vector<int> starts;
  vector<int> modes;
  RCPSPMAX * ins;
  AMOPBEncoding  amopbenc;

public:

	RCPSPMAXEncoding(RCPSPMAX * instance, AMOPBEncoding amopbenc);
	~RCPSPMAXEncoding();

	int getObjective() const;
	void getModes(vector<int> &modes);
	bool printSolution(ostream & os) const;

	virtual SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	virtual bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	void assumeBounds(const EncodedFormula & ef, int LB, int ub, vector<literal> & assumptions);

};

#endif

