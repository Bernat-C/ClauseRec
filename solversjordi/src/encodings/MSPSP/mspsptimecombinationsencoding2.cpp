#include "mspsptimecombinationsencoding2.h"
#include <set>
#include "util.h"

using namespace smtapi;


MSPSPTimeCombinationsEncoding2::MSPSPTimeCombinationsEncoding2(MSPSP * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs) : MSPSPEncoding(instance) {
	this->sargs = sargs;
	this->pargs = pargs;
}

MSPSPTimeCombinationsEncoding2::~MSPSPTimeCombinationsEncoding2() {

}

/*

Aquest encoding codifica totes les combinacions de igualtats/desigualtats de l'assignacio de recursos a activitats
A l'hora de definir a gerarquia de variables de l'estil 'ars', el conjunt c està implicat per qualsevol conjunt c \ {x}, sigui x qualsevol variabe de c
Tambe tenim que ars_c1 /\ ars_c2 -> ars_{c1 (intersec)  c2}
*/

SMTFormula * MSPSPTimeCombinationsEncoding2::encode(int lb, int ub){
	int N = ins->getNActivities();
	int R = ins->getNResources();
	int L = ins->getNSkills();

    
    SMTFormula * f = new SMTFormula();
    


	//Start time integer variables
	vector<intvar> S(N+2);
	for (int i=0;i<=N+1;i++){
		S[i]=f->newIntVar("S",i);
		if(1 <= i && i <= N){
			f->addClause(S[i] >= ins->ES(i));
			f->addClause(S[i] <= ins->LS(i,ub));
		}
	}

	//Activity 0 starts at time 0
	f->addClause(S[0] == 0);

	//Set bounds on makespan
	f->addClause(S[N+1] >= lb);
	f->addClause(S[N+1] <= ub);


	//Precedences
	for(int i = 0; i<= N+1; i++)
		for(int j =0; j <= N+1; j++)
			if(ins->isPred(i,j))
				f->addClause(S[j]-S[i] >= ins->getExtPrec(i,j));

	int nSubsetsOfSkills = 1 << L;
	//All combinations organized by cardinality
	vector<vector<int> >combinationsCard(L+1);
	for(int c = 0; c < nSubsetsOfSkills; c++)
		combinationsCard[util::cardinality(c)].push_back(c);

	vector<vector<bool> > resourceUsefulToSubset(R,vector<bool>(nSubsetsOfSkills,false));
	for(int k = 0; k < R; k++){
		for(int c = 0; c < nSubsetsOfSkills; c++){
			for(int l = 0; l < L; l++){
				if(util::nthBit(c,l) && ins->hasSkill(k,l)){
					resourceUsefulToSubset[k][c]=true;
					break;
				}
			}
		}
	}


	//Encoding of what subset of skills a resource is working on each activity
	for(int i = 1; i <= N; i++){

		//For each resource
		for(int k = 0; k < R; k++) if(ins->usefulToActivity(k,i)){

			vector<int> realSets; //Sets that have a real boolean variable associated (not just an alias)

			//For each subset of skills cardinality, skipping empty set
			for(int card = 1; card <= L; card++){
				for(int c : combinationsCard[card]){
					int comb = c;
					//For each skill in the subset
					for(int l = 0; l < L; l++) {
						//If not demanded or not mastered, remove from the subset
						if(!ins->hasSkill(k,l) || !ins->demandsSkill(i,l))
							comb&=~(1<<l);
					}
					//If empty set, subset of skills not useful for pair activity and resource
					if(comb!=0){
						//If some skill missing, alias to already existing variable
						if(comb!=c){
							f->aliasBoolVar(f->bvar("act_res_skills",i,k,comb),"act_res_skills",i,k,c);
						}

						//Otherwise, create variable, and define it.
						//The variables for just one skill are just created
						else if(card==1){
							f->newBoolVar("act_res_skills",i,k,c);
						}
		
						//Sets of more than one skill are defined from the subsets. 
						//Can be done in many ways. Here consider all subsets of cardinality one minus.
						else{
							if(pargs->getBoolOption(INTERSECTIONS))
								realSets.push_back(c);

							boolvar xc = f->newBoolVar("act_res_skills",i,k,c);
							clause cl = !xc;

							int element = 1;
							while(element&c==0) //Seek first element
								element <<=1;

							while(element <= c){ //While not elements not examined
								int subset = c&(~element);
								boolvar v = f->bvar("act_res_skills",i,k,subset);
								f->addClause(!v | xc);
								cl |= v;

								do element <<=1;
								while((element&c==0) && element <= c);//Seek next element
							}
							
							f->addClause(cl);

						}
					}
					
				}
			}

			//Compute intersections
			if(pargs->getBoolOption(INTERSECTIONS)){
				for(int ii = 0; ii < realSets.size(); ii++){
					int set1 = realSets[ii];
					for(int jj = ii+1; jj < realSets.size(); jj++){
						int set2 = realSets[jj];
						int intersec = set1 & set2;
						if(intersec != set1 && intersec != set2){
							if(intersec == 0){ //Intersection is the empty set, due to AMO constraints this is not allowed
								f->addClause(
									!f->bvar("act_res_skills",i,k,set1) 
									| !f->bvar("act_res_skills",i,k,set2)
								);
							}
							else{ //Channel the intersecion variable
								f->addClause(
									!f->bvar("act_res_skills",i,k,set1) 
									| !f->bvar("act_res_skills",i,k,set2) 
									| f->bvar("act_res_skills",i,k,intersec)
								);
							}
						}
					}
				}
			}


			//Alias to set of all skills to f->boolVar("act_res",i,k). Only if resource can perform some of the activities
			if(ins->usefulToActivity(k,i))
				f->aliasBoolVar(f->bvar("act_res_skills",i,k,nSubsetsOfSkills-1),"act_res",i,k);

			//Each resource spends at most one skill in each activity
			vector<literal> v;
			for(int l = 0; l < L; l++)
				if(ins->hasSkill(k,l) && ins->demandsSkill(i,l))
					v.push_back(f->bvar("act_res_skills",i,k,1<<l));
			f->addAMO(v,sargs->getAMOEncoding());
		}
	}


	//Each activity has exactly K resources covering a subset of skills with demand K
	//For each activity
	for(int i = 1; i <= N; i++){
		//For each subset of skills
		for(int c = 1; c < nSubsetsOfSkills; c++){
			bool allrequired = true;
			int demand = 0;
			for(int l = 0; l < L; l++)
			if(util::nthBit(c,l)){
				demand+=ins->getDemand(i,l);
				if(!ins->demandsSkill(i,l)){
					allrequired = false;
					break;
				}
			}


			//If all the skills of the subset c are required by activity i (non-dominated constraint)
			if(allrequired){
				vector<literal> v;
				for(int k = 0; k < R; k++)
					if(resourceUsefulToSubset[k][c])
						v.push_back(f->bvar("act_res_skills",i,k,c));
				
				if(pargs->getBoolOption(FULL))
					f->addEK(v,demand);
				else
					f->addALK(v,demand,sargs->getCardinalityEncoding());
			}
		}
	}


	//If implied constraint required, we introduce variable act_time
	if(pargs->getBoolOption(IMPLIED1) || pargs->getBoolOption(IMPLIED2) || pargs->getBoolOption(IMPLIED3)){
		for(int t = 0; t < ub; t++){
			for(int i = 1; i <= N; i++){
				if(ins->ES(i) <= t && t < ins->LC(i,ub)){
					boolvar x = f->newBoolVar("act_time",i,t);
					literal started = S[i] <= t;
					literal noended = t - ins->getDuration(i) < S[i];
                    f->addClause(!x | started);
                    f->addClause(!x | noended);
                    f->addClause(x | !started | !noended);
				}
			}
		}
	}

	//Each resource works in at most one activity at a time
	for(int k = 0; k < R; k++){
		for(int t = 0; t < ub; t++){
			vector<literal> v;
			for(int i = 1; i <= N; i++){
				if(ins->usefulToActivity(k,i) && ins->ES(i) <= t && t < ins->LC(i,ub)){
					boolvar x = f->newBoolVar("act_res_time",i,k,t);
					v.push_back(x);

					//Chanelling

					//If we use some implied, we can use variables act_time
					if(pargs->getBoolOption(IMPLIED1)||pargs->getBoolOption(IMPLIED2)||pargs->getBoolOption(IMPLIED3)){
						f->addClause(!x | f->bvar("act_time",i,t));
						f->addClause(!x | f->bvar("act_res",i,k));
						f->addClause(x | !f->bvar("act_res",i,k) | !f->bvar("act_time",i,t));
					}

					else{
						literal started = S[i] <= t;
						literal noended = t - ins->getDuration(i) < S[i];
						f->addClause(!x | started);
						f->addClause(!x | noended);
						f->addClause(!x | f->bvar("act_res",i,k));
						f->addClause(x | !started | !noended | !f->bvar("act_res",i,k));
					}

				}
			}
			f->addAMO(v,sargs->getAMOEncoding());
		}
	}



	//Implied 1: the number of activities requiring skill 'l' at a particular time
	//	is not greater than the number of resources that master 'l'
	if(pargs->getBoolOption(IMPLIED1)) for(int l = 0; l < L; l++){
		for(int t = 0; t < ub; t++){
			vector<vector<literal> > X;
			vector<vector<int> > Q;

			vector<int> vtasks;
			vector<set<int> > groups;

			for (int i=1;i<=N;i++) {
				int ESi=ins->ES(i);
				int LCi=ins->LC(i,ub);
				if (t>=ESi && t<LCi && ins->demandsSkill(i,l))
					vtasks.push_back(i);
			}

			if(!vtasks.empty())
				ins->computeMinPathCover(vtasks,groups);


			for (const set<int> & group : groups) {
				vector<literal> vars_part;
				vector<int> coefs_part;

				for(int i : group){
					vars_part.push_back(f->bvar("act_time",i,t));
					coefs_part.push_back(ins->getDemand(i,l));
				}

				if(!coefs_part.empty()){
					X.push_back(vars_part);
					Q.push_back(coefs_part);
				}
			}


			if (!X.empty()){
				util::sortCoefsDecreasing(Q,X);
				f->addAMOPB(Q,X,ins->getNResourcesMastering(l),sargs->getAMOPBEncoding());
			}
		}
	}


	//Implied 2: the number of activities running at a particular time
	//	is not greater than the number of resources
	if(pargs->getBoolOption(IMPLIED2)) for(int t = 0; t < ub; t++){

		vector<vector<literal> > X;
		vector<vector<int> > Q;

		vector<int> vtasks;
		vector<set<int> > groups;

		for (int i=1;i<=N;i++) {
			int ESi=ins->ES(i);
			int LCi=ins->LC(i,ub);
			if (t>=ESi && t<LCi)
				vtasks.push_back(i);
		}

		if(!vtasks.empty())
			ins->computeMinPathCover(vtasks,groups);

		for (const set<int> & group : groups) {
			vector<literal> vars_part;
			vector<int> coefs_part;

			for(int i : group){
				vars_part.push_back(f->bvar("act_time",i,t));
				coefs_part.push_back(ins->getTotalDemand(i));
			}

			if(!coefs_part.empty()){
				X.push_back(vars_part);
				Q.push_back(coefs_part);
			}
		}


		if (!X.empty()){
			util::sortCoefsDecreasing(Q,X);
			f->addAMOPB(Q,X,R,sargs->getAMOPBEncoding());
		}
	}


	//Implied 3: the number of activities running at a particular time
	//	is not greater than the number of resources available to cover
	//	any subset of skills
	// There is a dominance precomputation to avoid the introduction
	// of dominated constraints
	if(pargs->getBoolOption(IMPLIED3)){
		int nImplied = 0;

		//Compute dominances
		vector<bool> dominated(nSubsetsOfSkills,false);

		vector<int> capacity(nSubsetsOfSkills,0);
		for(int combination = 1; combination < nSubsetsOfSkills; combination++){
			for(int k = 0; k < R; k++){
				for(int l = 0; l < ins->getNSkills(); l++)
				if(util::nthBit(combination,l) && ins->hasSkill(k,l)){
					capacity[combination]++;
					break;
				}
			}
		}

		//For each non-dominated combination of skills
		for(int combination = 0; combination < nSubsetsOfSkills; combination++){ //Has to be read as a binary number
			for(int c2 = 1; c2 < nSubsetsOfSkills; c2++)
			if(c2 != combination && capacity[c2]==capacity[combination]){
				bool superset = true;
				for(int l = 0; l < ins->getNSkills(); l++)
				if(util::nthBit(combination,l) && !util::nthBit(c2,l)){
					superset=false;
					break;
				}
				if(superset){
					dominated[combination]=true;
					break;
				}
			}
		}


		for(int t = 0; t < ub; t++){

			vector<int> vtasks;
			vector<set<int> > groups;

			vector<bool> required(L,false);

			for (int i=1;i<=N;i++){
				int ESi=ins->ES(i);
				int LCi=ins->LC(i,ub);
				if (t>=ESi && t<LCi){
					vtasks.push_back(i);
					for(int l = 0; l < ins->getNSkills(); l++)
						if(ins->demandsSkill(i,l))
							required[l] = true;
				}
			}

			if(!vtasks.empty())
				ins->computeMinPathCover(vtasks,groups);


			//At each time instant, only consider the subsets that contain skills required 
			//by some of the activities that can be running at this particular time
			
			

			//For each non-dominated combination of skills
			for(int combination = 1; combination < nSubsetsOfSkills; combination++){ //Has to be read as a binary number
				bool allrequired = true;
				for(int l = 0; l < L; l++)
				if(util::nthBit(combination,l) && !required[l]){
					allrequired = false;
					break;
				}
		
				//If every skill is required and the combination is not dominated
				if(allrequired && !dominated[combination])
				{
					vector<vector<literal> > X;
					vector<vector<int> > Q;
					for (const set<int> & group : groups) {
						vector<literal> vars_part;
						vector<int> coefs_part;

						for(int i : group){
							int demand = 0;
							for(int l = 0; l < ins->getNSkills(); l++)
								if(util::nthBit(combination,l))
									demand += ins->getDemand(i,l);

							if(demand != 0){
								vars_part.push_back(f->bvar("act_time",i,t));
								coefs_part.push_back(demand);
							}
						}

						X.push_back(vars_part);
						Q.push_back(coefs_part);
					}


					if (!X.empty()){
						nImplied++;
						f->addAMOPB(Q,X,capacity[combination],sargs->getAMOPBEncoding());
					}
				}
			}
		}
		cout << "c nImpliedAsserted is " << nImplied << "/" << (ub * (1<<ins->getNSkills())) << endl;
	}


	//Symetry breaking: set an order on the use of identical resources
	if(pargs->getBoolOption(SYMBREAK)) for(int t = 0; t < ub; t++){
		//Introduce variable "res_time"
		for(int k = 0; k < R; k++){
			boolvar v = f->newBoolVar("res_time",k,t);
			clause c = !v;
			for(int i = 1; i <= N; i++){
				if(ins->usefulToActivity(k,i) && ins->ES(i) <= t && t < ins->LC(i,ub)){
					f->addClause(!f->bvar("act_res_time",i,k,t) | v);
					c |= f->bvar("act_res_time",i,k,t);
				}
			}
			f->addClause(c);
		}

		for(int type = 0; type < ins->getNResourceTypes(); type++){
			const vector<int> & res_type = ins->getResourcesOfType(type);
			int nOfType = res_type.size();
			for(int ri = 0; ri < nOfType-1; ri++){
				for(int i = 1; i <= N; i++) 
				if(ins->usefulToActivity(res_type[ri],i) && ins->ES(i) <= t && t <= ins->LS(i,ub)){
					for(int rj = ri+1; rj < nOfType; rj++){
						//If resource  'res_type[ri]'  starts to work at an activity at time 't', 
						// then every resource 'res_type[rj]' of the same type s.t  'rj' precedes 'ri' has to be occupied
						f->addClause(
							(t-1 < ins->ES(i) ? f->falseVar() : f->bvar("act_res_time",i,res_type[ri],t-1)) |
							!f->bvar("act_res_time",i,res_type[ri],t) |
							f->bvar("res_time",res_type[rj],t)
						);
					}
				}
			}
		}
	}
	return f;
}

void MSPSPTimeCombinationsEncoding2::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();
	int R = ins->getNResources();
	int L = ins->getNSkills();

	this->starts=vector<int>(N+2);
	this->assignment.clear();
	this->assignment.resize(N+2);

	for (int i=0;i<=N+1;i++){
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
		for(int l = 0; l < L; l++){
			int demand = ins->getDemand(i,l);

			for(int k = 0; k < R; k++){
				if(ins->hasSkill(k,l) && ins->demandsSkill(i,l)){
					if(SMTFormula::getBValue(ef.f->bvar("act_res_skills",i,k,1<<l),bmodel)){
						if(demand > 0){
							assignment[i].push_back(pair<int,int>(k,l));
							demand--;
						}
					}
				}
			}
		}
	}
}

bool MSPSPTimeCombinationsEncoding2::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();
	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		return true;
	}
	else return false;
}

void MSPSPTimeCombinationsEncoding2::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}