#ifndef YICES2APIENCODER_DEFINITION
#define YICES2APIENCODER_DEFINITION

#include "apiencoder.h"
#include <yices.h>

using namespace smtapi;


/*
 * This class asserts an SMT formula to the Yices2 API.
 */
class Yices2APIEncoder : public APIEncoder {

private:

	//Configuration parameters
	double randomseed;

	std::vector<term_t> boolvars;
	std::vector<term_t> intvars;

	int lastBoolvar;
	int lastIntvar;
	int lastClause;

	type_t int_type;
	type_t bool_type;

	static bool yicesIsInitialized;


	ctx_config_t *yconfig;
	param_t * param;
	context_t * ctx;

	static void initialize_yices();

	bool assertAndCheck(int lb, int ub, std::vector<literal> * assumptions);

	//Returns the term_t representation of 's'
	term_t sumterm(const intsum & s, const std::vector<term_t> & intvars);

	//Returns the term_t representation of 'l'
	term_t getTerm(const literal & l, const std::vector<term_t> & boolvars, const std::vector<term_t> & intvars);


public:
	//Default constructor
	Yices2APIEncoder(Encoding * enc);

	//Destructor
	~Yices2APIEncoder();

	bool checkSAT(int lb, int ub);
	bool checkSATAssuming(int lb, int ub);
	void narrowBounds(int lb, int ub);

	context_t * getContext();

	void setRandomSeed(double seed);


};

#endif

