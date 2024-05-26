#include "yices2apiencoder.h"
#include "errors.h"
#include <iostream>

bool Yices2APIEncoder::yicesIsInitialized = false;

Yices2APIEncoder::Yices2APIEncoder(Encoding * enc) : APIEncoder(enc){

	this->lastBoolvar = 0;
	this->lastIntvar = 0;
	this->lastClause = -1;

	initialize_yices();

	int_type = yices_int_type();
	bool_type = yices_bool_type();

	yconfig = yices_new_config();
	yices_set_config(yconfig, "mode", "multi-checks");
	yices_default_config_for_logic(yconfig, "QF_LIA");

	param = yices_new_param_record();
	yices_set_param(param, "branching", "theory");//params->branching = BRANCHING_THEORY;
	yices_set_param(param, "cache-tclauses", "true");//params->cache_tclauses = true;
	yices_set_param(param, "tclause-size", "20");//params->tclause_size = 8;
	yices_set_param(param, "simplex-prop", "true");//params->use_simplex_prop = true;
	randomseed = -1;

	ctx=yices_new_context(yconfig);

	workingFormula = EncodedFormula();

}

Yices2APIEncoder::~Yices2APIEncoder(){
	yices_reset();
}

void Yices2APIEncoder::initialize_yices(){
	if(!yicesIsInitialized){
		yices_init();
		yicesIsInitialized = true;
	}
}

bool Yices2APIEncoder::checkSATAssuming(int lb, int ub){

	std::vector<literal> assumptions;
	enc->assumeBounds(workingFormula,lb,ub,assumptions);


	return assertAndCheck(lb,ub,&assumptions);
}

void Yices2APIEncoder::narrowBounds(int lb, int ub){
	enc->narrowBounds(workingFormula,lastLB,lastUB,lb,ub);
}

bool Yices2APIEncoder::checkSAT(int lb, int ub){
	if(workingFormula.f==NULL){
		workingFormula = EncodedFormula(enc->encode(lb, ub),lb,ub);
	}
	else{
		bool narrowed = enc->narrowBounds(workingFormula,lastLB, lastUB, lb, ub);
		if(!narrowed){
			delete workingFormula.f;
			this->lastBoolvar = 0;
			this->lastIntvar = 0;
			this->lastClause = -1;
			workingFormula = EncodedFormula(enc->encode(lb, ub),lb,ub);
			yices_reset_context(ctx);
			boolvars.clear();
			intvars.clear();
		}
	}
	lastLB = lb;
	lastUB = ub;

	return assertAndCheck(lb,ub,NULL);
}

bool Yices2APIEncoder::assertAndCheck(int lb, int ub, std::vector<literal> * assumptions){

	//Add the new variables
	boolvars.resize(workingFormula.f->getNBoolVars()+1);
	intvars.resize(workingFormula.f->getNIntVars()+1);

	for(int i = lastBoolvar+1; i <= workingFormula.f->getNBoolVars(); i++)
		boolvars[i]=yices_new_uninterpreted_term(bool_type);
	for(int i = lastIntvar+1; i <= workingFormula.f->getNIntVars(); i++)
		intvars[i]=yices_new_uninterpreted_term(int_type);


	//Add the new clause
	for(int i = lastClause+1; i < workingFormula.f->getNClauses(); i++){
		const clause & c = workingFormula.f->getClauses()[i];
		std::vector<term_t> terms;
		for(const literal & l : c.v)
			terms.push_back(getTerm(l,boolvars,intvars));
		if(terms.empty()) //Empty clause
			yices_assert_formula(ctx,yices_false());
		else{
			int code = yices_assert_formula(ctx,yices_or(terms.size(),&terms[0]));
			if(code == -1){
				fprintf(stderr,"A constraint couldn't be asserted to the SMT solver. Aborting with error code %d\n.",yices_error_code());
				yices_print_error(stderr);
				exit(BADCODIFICATION_ERROR);
			}
		}
	}

	lastBoolvar = workingFormula.f->getNBoolVars();
	lastIntvar = workingFormula.f->getNIntVars();
	lastClause = workingFormula.f->getNClauses()-1;


	clock_t begin_time = clock();

	//Make the satisfiability check
	bool sat;
	if(assumptions==NULL)
		sat=yices_check_context(ctx,param)== STATUS_SAT;
	else{
		int nassumptions = assumptions->size();
		term_t * assumptions_t = new term_t[nassumptions];
		for(int i = 0; i < nassumptions; i++)
			assumptions_t[i] = getTerm((*assumptions)[i],boolvars,intvars);
		sat=yices_check_context_with_assumptions(ctx,param,nassumptions,assumptions_t)== STATUS_SAT;
		delete [] assumptions_t;
	}

	lastchecktime = float( clock() - begin_time ) /  CLOCKS_PER_SEC;

	//Retrieve the model
	if(sat && produceModels()){
		model_t * mdl = yices_get_model(ctx,true);

		std::vector<int> imodel(workingFormula.f->getNIntVars()+1);
		std::vector<bool> bmodel(workingFormula.f->getNBoolVars()+1);

		for(int i = 1; i <= workingFormula.f->getNBoolVars(); i++){
			int aux;
			yices_get_bool_value(mdl,boolvars[i],&aux);
			bmodel[i]= aux==1;
		}
		for(int i = 1; i <= workingFormula.f->getNIntVars(); i++){
			int aux;
			yices_get_int32_value(mdl,intvars[i],&aux);
			imodel[i]=aux;
		}
		yices_free_model(mdl);

		enc->setModel(workingFormula,lb,ub,bmodel,imodel);

	}

	//Retrieve statistics
#ifdef CUSTOMYICES
	natoms = custom_get_natoms(ctx);
	nrestarts = custom_get_nrestarts(ctx);
	nsimplify = custom_get_nsimplify(ctx);
	nreduce = custom_get_nreduce(ctx);
	nsimplify = custom_get_nsimplify(ctx);
	nreduce = custom_get_nreduce(ctx);
	ndecisions = custom_get_ndecisions(ctx);
	npropagations = custom_get_npropagations(ctx);
	nconflicts = custom_get_nconflicts(ctx);
	ntheorypropagations = custom_get_ntheorypropagations(ctx);
	ntheoryconflicts = custom_get_ntheoryconflicts(ctx);
#endif

	return sat;
}

term_t Yices2APIEncoder::getTerm(const literal & l, const std::vector<term_t> & boolvars, const std::vector<term_t> & intvars){
	term_t term;
	if(l.arith){
		term = sumterm(l.cmp.s,intvars);//Lhs sum
		if(l.cmp.eq) //Equality ==
			term = yices_arith_eq_atom(term,yices_int32(l.cmp.k));
		else //Inequality <=
			term = yices_arith_leq_atom(term,yices_int32(l.cmp.k));
	}
	else{
		if(l.v.id<=0 || l.v.id > workingFormula.f->getNBoolVars()){
			std::cerr << "Error: asserted undefined Boolean variable: " << l.v.id << std::endl;
			exit(UNDEFINEDVARIABLE_ERROR);
		}
		else
			term = boolvars[l.v.id];
	}
	if(!l.sign)
		term=yices_not(term);

	return term;
}

term_t Yices2APIEncoder::sumterm(const intsum & s, const std::vector<term_t> & intvars){
	if(s.v.empty())
		return yices_zero();
	else{
		std::vector<term_t> summands;
		for(const intprod & p : s.v){
			if(p.varid <= 0 || p.varid > workingFormula.f->getNIntVars()){
				std::cerr << "Error: asserted undefined Int variable"<< std::endl;
				exit(UNDEFINEDVARIABLE_ERROR);
			}
			else
				summands.push_back(yices_mul(yices_int32(p.coef),intvars[p.varid]));
		}
		return yices_sum(summands.size(),&summands[0]);
	}
}

context_t * Yices2APIEncoder::getContext(){
	return ctx;
}

void Yices2APIEncoder::setRandomSeed(double seed){
	randomseed = seed;
	char s_seed [20];
	sprintf(s_seed,"%d",randomseed);
	yices_set_param(param,"random-seed",s_seed);
}
