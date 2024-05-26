#ifndef MINISATAPIENCODER_DEFINITION
#define MINISATAPIENCODER_DEFINITION

#include "apiencoder.h"
#include "maple/core/Solver.h"
#include "maple/core/SolverTypes.h"
#include "maple/mtl/Vec.h"

using namespace smtapi;

/*
 * This class asserts an SMT formula to the Yices2 API.
 */
class MinisatAPIEncoder : public APIEncoder {

private:

	Minisat::Solver * s;

	std::vector<Minisat::Var> vars; // TODO remove.
    
    int trace_sat;
    bool restarts_enabled;
    int phase_saving;

	bool use_gnn;
	std::string instance_name;
	int time_lost;

	int lastVar;
	int lastClause;

	std::vector<std::vector<float>> features;

	Minisat::Lit getLiteral(const literal & l, const std::vector<Minisat::Var> & boolvars);
	bool assertAndCheck(int lb, int ub, std::vector<literal> * assumptions);

public:
	//Default constructor
	MinisatAPIEncoder(Encoding * enc, int trace_sat, bool restarts_enabled, int phase_saving, std::string fileprefix, bool use_gnn = false);

	//Destructor
	~MinisatAPIEncoder();

	bool checkSAT(int lb, int ub);
	bool checkSATAssuming(int lb, int ub);
	void narrowBounds(int lb, int ub);

	void toFeatureFile (const char *file);
    void toFeatureFile (FILE* f);
};

#endif

