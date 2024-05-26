#ifndef PRCPSPENCODING_DEFINITION
#define PRCPSPENCODING_DEFINITION

#include <vector>
#include "prcpsp.h"
#include "smtformula.h"
#include "encoding.h"


using namespace std;

class PRCPSPEncoding : public Encoding {
private:

protected:

  vector<int> starts;
  vector<vector<bool>> en_execucio;
  PRCPSP * ins;

public:

	PRCPSPEncoding(PRCPSP * instance);
	int getObjective() const;
	void getStarts(vector<int> &starts);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	bool printSolution(ostream & os) const;
	virtual ~PRCPSPEncoding();
};

#endif

