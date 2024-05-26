#include "minisatapiencoder.h"
#include "errors.h"
#include <iostream>

using namespace Minisat;

MinisatAPIEncoder::MinisatAPIEncoder(Encoding * enc, int trace_sat, bool restarts_enabled, int phase_saving, std::string fileprefix, bool use_gnn) : APIEncoder(enc){

	this->lastVar = 0;
	this->lastClause = -1;
    
    this->trace_sat = trace_sat;
    this->restarts_enabled = restarts_enabled;
    this->phase_saving = phase_saving;
	
	this->instance_name = fileprefix.substr(fileprefix.find_last_of("/\\") + 1);

	this->use_gnn = use_gnn;
	
	//FILE* df = fopen(str.c_str(),"wr");

	s = new Solver();
	//s->drup_file = df;
	s->verbosity=0;
    s->random_var_freq=-1;
    //s->print_trace=trace_sat;
    //s->restarts_enabled = restarts_enabled;
    s->phase_saving = phase_saving;
	s->instance_name = this->instance_name.c_str();
	s->clause_rec = use_gnn;
	

}

MinisatAPIEncoder::~MinisatAPIEncoder(){
	delete s;
}

bool MinisatAPIEncoder::checkSATAssuming(int lb, int ub){

	std::vector<literal> assumptions;
	enc->assumeBounds(workingFormula,lb,ub,assumptions);

	return assertAndCheck(lb,ub,&assumptions);
}

void MinisatAPIEncoder::narrowBounds(int lb, int ub){
	enc->narrowBounds(workingFormula,lastLB,lastUB,lb,ub);
}

bool MinisatAPIEncoder::checkSAT(int lb, int ub){
	if(workingFormula.f==NULL){
		workingFormula = EncodedFormula(enc->encode(lb, ub),lb,ub);
	}
	else{
		bool narrowed = enc->narrowBounds(workingFormula,lastLB, lastUB, lb, ub);
		if(!narrowed){
			delete workingFormula.f;
			this->lastVar = 0;
			this->lastClause = -1;
			workingFormula = EncodedFormula(enc->encode(lb, ub),lb,ub);

			//Reset minisat solver
			delete s;
			s = new Solver();
            s->verbosity=0;
            s->random_var_freq=-1;
			s->instance_name = this->instance_name.c_str();
            // s->print_trace=trace_sat;
            // s->restarts_enabled = restarts_enabled;
            s->phase_saving = phase_saving;//phase_saving;
			s->clause_rec = use_gnn;
			vars.clear();
		}
		else {
			this->lastVar = 0;
			this->lastClause = -1;
			delete s;
			s = new Solver();
            s->verbosity=0;
            s->random_var_freq=-1;
			s->instance_name = this->instance_name.c_str();
			s->clause_rec = use_gnn;
            // s->print_trace=trace_sat;
            // s->restarts_enabled = restarts_enabled;
            s->phase_saving = phase_saving;
			vars.clear();
		}
	}

	lastLB = lb;
	lastUB = ub;

	return assertAndCheck(lb,ub,NULL);
}


bool MinisatAPIEncoder::assertAndCheck(int lb, int ub, std::vector<literal> * assumptions){

	//Add the new variables
	vars.resize(workingFormula.f->getNBoolVars()+1);
    
	features = workingFormula.f->getVarsFeatures();
	assert(features.size() == workingFormula.f->getNBoolVars());
	
	for(int i = lastVar+1; i<features.size(); i++) {
		vars[i]=s->newVar(true, !workingFormula.f->usePredefDecs());
	}
	        
    if(workingFormula.f->usePredefDecs()){
        if(workingFormula.f->usePredefOrder())
            std::cout << "c Using predef order!" << std::endl;
        else
            std::cout << "c Using predef decs!" << std::endl;
        std::vector<literal> vect;
        workingFormula.f->getPredefDecs(vect);
        
        vec<double> order;
        if(workingFormula.f->usePredefOrder()){
            order.growTo(workingFormula.f->getNBoolVars(),0);
        }
        
        int i = 0;
        for(const literal & l : vect){
            s->setDecisionVar(vars[l.v.id],true);
            
            //If use prederfined order, the most prioritary is a variable, the higher its order value
            if(workingFormula.f->usePredefOrder()){
                order[vars[l.v.id]] = workingFormula.f->getNBoolVars() - i;
                i++;
            }
        }
        
        //Not only set what variables can be used to decide, but an order over them
        /* if(workingFormula.f->usePredefOrder())
            s->setPredefOrder(order); */
        
    }

    /* if(this->trace_sat > 0)
        s->setVarNames(workingFormula.f->getBoolVarNames()); */
    
	bool consistent = true;

	//Add the new clauses
	for(int i = lastClause+1; i < workingFormula.f->getNClauses(); i++){
		const clause & c = workingFormula.f->getClauses()[i];
		vec<Lit> cv;
		for(const literal & l : c.v)
			cv.push(getLiteral(l,vars));
		if(!s->addClause(cv)){
			consistent = false;
			break;
		}
		consistent = s->simplify();
	}
	lastVar = workingFormula.f->getNBoolVars();
	lastClause = workingFormula.f->getNClauses()-1;

	bool sat;
	if(!consistent){
		sat=false;
		lastchecktime=0;
	}
	else{
		std::string filename;
		filename = this->instance_name + ".dimacs";// "auxiliar.dimacs";
		s->toDimacs(filename.c_str());

		std::string feature_file = this->instance_name + ".features"; // features.ft
		toFeatureFile(feature_file.c_str());

        //Set variables to decide (if option enabled)
        
		clock_t begin_time = clock();
        
		//Make the satisfiability check
		int nassumptions = assumptions==NULL ? 0 : assumptions->size();
		vec<Lit> dummy(nassumptions);

		for(int i = 0; i < nassumptions; i++)
			dummy[i]=getLiteral((*assumptions)[i],vars);

		sat = s->solveLimited(dummy)==l_True;

		lastchecktime = float( clock() - begin_time ) /  CLOCKS_PER_SEC;

	}

	//Retrieve the model
	if(sat && produceModels()){
		std::vector<bool> bmodel(workingFormula.f->getNBoolVars()+1, false);
		std::vector<int> imodel;
		
		int ntrue=0;
		int nfalse=0;
		for(int i = 1; i < workingFormula.f->getNBoolVars(); i++){
			//std::cout << (s->model[i-1]==l_True) << std::endl;
			bmodel[i]=s->model[i-1]==l_True;
			if(bmodel[i])ntrue++;
			else nfalse++;
		}
		enc->setModel(workingFormula,lb,ub,bmodel,imodel);
	}
    

    nrestarts = s->starts;
    ndecisions = s->decisions;
    npropagations = s->propagations;
    nconflicts = s->conflicts;
	time_lost = s->time_lost;
    
    std::cout << "c restarts " << nrestarts << std::endl;
    std::cout << "c decisions " << ndecisions << std::endl;
    std::cout << "c propagations " << npropagations << std::endl;
    std::cout << "c conflics " << nconflicts << std::endl;
    std::cout << "c time spent in gnn (ms)) " << time_lost << std::endl;
    std::cout << "c SAT? " << sat << " UB: " << ub << std::endl;
    
	return sat;
}

Lit MinisatAPIEncoder::getLiteral(const literal & l, const std::vector<Var> & vars){
	if(l.arith){
		assert(false);
		std::cerr << "Error: Minisat cannot deal with arithmetic literals"<< std::endl;
		exit(BADCODIFICATION_ERROR);
	}
	else{
		if(l.v.id<=0 || l.v.id > workingFormula.f->getNBoolVars()){
			std::cerr << "Error: asserted undefined Boolean variable: " << l.v.id << std::endl;
			exit(UNDEFINEDVARIABLE_ERROR);
		}
		else
			return mkLit(vars[l.v.id],!l.sign);
	}
}

void MinisatAPIEncoder::toFeatureFile(const char *file)
{
    FILE* f = fopen(file, "wr");
    if (f == NULL)
        fprintf(stderr, "could not open file %s\n", file), exit(1);
    toFeatureFile(f);
    fclose(f);
}

void MinisatAPIEncoder::toFeatureFile(FILE* f)
{        
	assert(s->map.size()<=features.size());
    for (int i = 0; i < s->map.size(); i++) {
		Var el = s->map[i];
		if (el != var_Undef) {
			fprintf(f,"%d",el+1);
			for (int j = 0; j < features.at(i).size(); j++) {
				float v = features.at(i)[j];
				fprintf(f," %6.4lf",v);
			}
			fprintf(f,"\n");
		}
    }
}
