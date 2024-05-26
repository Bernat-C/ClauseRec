#include "smttaskencoding.h"
#include "util.h"
#include "smtapi.h"

using namespace smtapi;

SMTTaskEncoding::SMTTaskEncoding(MRCPSP * instance, SolvingArguments * sargs, bool omt) : MRCPSPEncoding(instance)  {
	this->omt = omt;
	this->sargs = sargs;
}



SMTFormula * SMTTaskEncoding::encode(int lb, int ub){

	int N = ins->getNActivities();
    int N_RR = ins->getNRenewable();
    int N_NR = ins->getNNonRenewable();
	int N_R = ins->getNResources();

	SMTFormula * f = new SMTFormula();

	//Execution modes of the activities
	for (int i=0;i<=N+1;i++) {
		vector<literal> vmodes;
		for (int o=0;o<ins->getNModes(i);o++)
			vmodes.push_back(f->newBoolVar("sm",i,o));

		f->addEO(vmodes); //Each activity has exactly one execution mode
	}

	//Creation of z_i,j,o
	for (int j=1;j<=N;j++)
		for (int i=1;i<=N;i++)
			if(i!=j && !ins->inPath(i,j))
				for (int o=0;o<ins->getNModes(i);o++)
					f->newBoolVar("z",i,j,o);


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

	//Objective function if using OMT
	if(omt)
		f->minimize(S[N+1]);

	//Definition of x_i,j,o
	for (int j=1;j<=N;j++){
		for (int i=1;i<=N;i++){
			if(i!=j && !ins->inPath(i,j)){
				for (int o=0;o<ins->getNModes(i);o++){
					boolvar z = f->bvar("z",i,j,o);
					boolvar sm = f->bvar("sm",i,o);
					literal geSi = S[i] - S[j] <= 0;
					literal ltCi = S[j] - S[i] < ins->getDuration(i,o);

					f->addClause(!z | geSi);
					f->addClause(!z | ltCi);
					f->addClause(!z | sm);
					f->addClause(z | !geSi | !ltCi | !sm);
				}
			}
		}
	}

	//Precedence constraints
	for (int i=0;i<=N+1;i++) {
		for (int j=0;j<=N+1;j++) {

			/*if (i==j && preds[i][j]>0) //Self precedence
				f->addEmptyClause();

			if (i!=j && preds[i][j]!=INT_MIN && preds[j][i]>0)   //Cycle
				f->addEmptyClause();*/

			//Extended precedences. Implied constraint. WARNING: if removed, direct precedence constraints below must be uncommented
			if(ins->isPred(i,j))
				f->addClause(S[j] - S[i] >= ins->getExtPrec(i,j));
		}

		for (int j : ins->getSuccessors(i)) {
			int min=ins->getMinDuration(i);
			for (int k=0;k<ins->getNModes(i);k++) {
				if (min<ins->getDuration(i,k))
					f->addClause(!f->bvar("sm",i,k) | S[j] - S[i] >= ins->getDuration(i,k));
				/*else //Already done by extended precedences
					f->addClause(S[j] - S[i] >= ins->getDuration(k));*/
			}
		}
	}


	//Renewable resource constraints
	for (int r=0;r<N_RR;r++) {
		for (int j=1;j<=N;j++){

			vector<vector<literal> > X;
			vector<vector<int> > Q;

			vector<int> vtasques;
			vector<set<int> > groups;

			for (int i=1;i<=N;i++)
				if(i!=j && !ins->inPath(i,j))
					vtasques.push_back(i);

			if(!vtasques.empty())
				ins->computeMinPathCover(vtasques,groups);


			for (const set<int> & group : groups) {
				vector<literal> vars_part;
				vector<int> coefs_part;

				for(int i : group){
					for (int o=0;o<ins->getNModes(i);o++) {
						vars_part.push_back(f->bvar("z",i,j,o));
						coefs_part.push_back(ins->getDemand(i,r,o));
					}
				}
				X.push_back(vars_part);
				Q.push_back(coefs_part);
			}

			//Get the terms for activity 'j'
			vector<literal> vars_part;
			vector<int> coefs_part;
			int minCoef = INT_MAX;
			for (int o=0;o<ins->getNModes(j);o++) {
				int q = ins->getDemand(j,r,o);
				if(q < minCoef)
					minCoef=q;
				coefs_part.push_back(q);
				vars_part.push_back(f->bvar("sm",j,o));
			}
			for(int & coef : coefs_part)
				coef-=minCoef;

			X.push_back(vars_part);
			Q.push_back(coefs_part);

			f->addAMOPB(Q,X,ins->getCapacity(r)-minCoef,sargs->getAMOPBEncoding());
		}
		
	}


	//Non-renewable resource constraints
	for (int r=N_RR;r<N_R;r++) {

		vector<vector<literal> > X;
		vector<vector<int> > Q;
		for (int j=1;j<=N;j++) {
			vector<literal> vars_part;
			vector<int> coefs_part;
			for (int g=0;g<ins->getNModes(j);g++) {
				vars_part.push_back(f->bvar("sm",j,g));
				coefs_part.push_back(ins->getDemand(j,r,g));
			}
			X.push_back(vars_part);
			Q.push_back(coefs_part);
		}

		f->addAMOPB(Q,X,ins->getCapacity(r),sargs->getAMOPBEncoding());
		
	}

	return f;
}

void SMTTaskEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();


	this->starts=vector<int>(N+2);
	this->modes=vector<int>(N+2);
	for (int i=0;i<=N+1;i++){
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
		for (int p=0;p<ins->getNModes(i);p++){
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel)){
				this->modes[i]=p;
				break;
			}
		}
	}
}

bool SMTTaskEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		return true;
	}
	else return false;
}

void SMTTaskEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}

SMTTaskEncoding::~SMTTaskEncoding() {

}
