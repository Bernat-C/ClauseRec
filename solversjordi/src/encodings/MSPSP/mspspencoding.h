#ifndef MSPSPENCODING_DEFINITION
#define MSPSPENCODING_DEFINITION

#include <vector>
#include "mspsp.h"
#include "smtformula.h"
#include "encoding.h"


using namespace std;
using namespace smtapi;

/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	IMPLIED1,
	IMPLIED2,
	IMPLIED3,
	ENERGY,
	SYMBREAK,
	UPPER,
	CPOPT,
	OMT,
	UNIT,
	GERARQUIC,
	PRINT,
	PRINTDZN,
	PRINTBOUNDS,
	FULL,
	ENCODING,
	INTERSECTIONS,
	JSON,
	LP_CT_REAL,
	LP_CT_INT,
	LP_DT,
	LP_DDT,
	LP_JC_REAL,
	LP_JC_INT,
	LP_HR
};



class MSPSPEncoding : public Encoding {
private:

protected:

  vector<int> starts;
  vector<vector<pair<int, int> > > assignment;

  bool implied1;
  bool implied2;

  AMOPBEncoding amopbencoding;
  MSPSP * ins;

  void assertNBDD(NBDD * nb, const vector<literal> & selectors, SMTFormula * f) const;

public:

	MSPSPEncoding(MSPSP * instance);
	~MSPSPEncoding();

	int getObjective() const;
	bool printSolution(ostream & os) const;

	void assumeBounds(const EncodedFormula & ef, int LB, int ub, vector<literal> & assumptions);


};

#endif

