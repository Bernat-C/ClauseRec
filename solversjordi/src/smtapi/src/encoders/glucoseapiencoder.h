#ifndef GLUCOSEAPIENCODER_DEFINITION
#define GLUCOSEAPIENCODER_DEFINITION

#include "apiencoder.h"
#include "glucose/simp/SimpSolver.h"
#include "glucose/core/SolverTypes.h"
#include "glucose/mtl/Vec.h"

using namespace smtapi;
using namespace Glucose;


/*
 * This class asserts an SMT formula to the Yices2 API.
 */
class GlucoseAPIEncoder : public APIEncoder {

private:

	Glucose::Solver * s;

	std::vector<Glucose::Var> vars;

	int lastVar;
	int lastClause;

	Glucose::Lit getLiteral(const literal & l, const std::vector<Glucose::Var> & boolvars);
	bool assertAndCheck(int lb, int ub, std::vector<literal> * assumptions);

public:
	//Default constructor
	GlucoseAPIEncoder(Encoding * enc);

	//Destructor
	~GlucoseAPIEncoder();

	bool checkSAT(int lb, int ub);
	bool checkSATAssuming(int lb, int ub);
	void narrowBounds(int lb, int ub);

};

#endif

