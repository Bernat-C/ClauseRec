#include "mspsptaskcombinationsencoding2.h"
#include <set>
#include "util.h"

using namespace smtapi;

MSPSPTaskCombinationsEncoding2::MSPSPTaskCombinationsEncoding2(MSPSP * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs) : MSPSPEncoding(instance) {
	this->sargs = sargs;
	this->pargs = pargs;
}

MSPSPTaskCombinationsEncoding2::~MSPSPTaskCombinationsEncoding2() {

}


SMTFormula * MSPSPTaskCombinationsEncoding2::encode(int lb, int ub){
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
					f->addALK(v,demand);
			}
		}
	}

	//Definition of o_{i,j}
	for (int j=1;j<=N;j++){
		for (int i=1;i<=N;i++){
			if(i!=j && !ins->inPath(i,j)){
				boolvar o = f->newBoolVar("o",i,j);
				literal geSi = S[i] - S[j] <= 0;
				literal ltCi = S[j] - S[i] < ins->getDuration(i);

				f->addClause(!o | geSi);
				f->addClause(!o | ltCi);
				f->addClause( o | !geSi | !ltCi);
			}
		}
	}

	//No resource is working on two activities at a time
	for(int k = 0; k < R; k++)
		for(int j=1;j<=N;j++)
			for(int i=1;i<=N;i++)
				if(i!=j && !ins->inPath(i,j))
					f->addClause(!f->bvar("act_res",i,k) | !f->bvar("act_res",j,k) | !f->bvar("o",i,j));



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


		for(int j=1;j<=N;j++){

			vector<int> vtasks;
			vector<set<int> > groups;

			vector<bool> required(L,false);
			for(int l = 0; l < ins->getNSkills(); l++)
				if(ins->demandsSkill(j,l))
					required[l]=true;

			for (int i=1;i<=N;i++) if(i!=j && !ins->inPath(i,j)){
				vtasks.push_back(i);
				for(int l = 0; l < ins->getNSkills(); l++)
					if(ins->demandsSkill(i,l))
						required[l] = true;
			}

			if(!vtasks.empty())
				ins->computeMinPathCover(vtasks,groups);
			

			//For each non-dominated combination of skills
			for(int combination = 1; combination < nSubsetsOfSkills; combination++){ //Has to be read as a binary number
				bool allrequired = true;
				int demandJ=false;
				for(int l = 0; l < L; l++){
					if(util::nthBit(combination,l)){
						if(required[l])
							demandJ+=ins->getDemand(j,l);
						else{
							allrequired = false;
							break;
						}
					}
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

							vars_part.push_back(f->bvar("o",i,j));
							coefs_part.push_back(demand);
						}

						X.push_back(vars_part);
						Q.push_back(coefs_part);
					}



					if (!X.empty()){
						nImplied++;
						f->addAMOPB(Q,X,capacity[combination]-demandJ,sargs->getAMOPBEncoding());
					}
				}
			}
		}
		cout << "c nImpliedAsserted is " << nImplied << "/" << (ub * (1<<ins->getNSkills())) << endl;
	}


	return f;
}

void MSPSPTaskCombinationsEncoding2::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
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

bool MSPSPTaskCombinationsEncoding2::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();
	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		return true;
	}
	else return false;
}


void MSPSPTaskCombinationsEncoding2::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}