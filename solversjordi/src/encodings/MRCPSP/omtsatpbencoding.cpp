#include "omtsatpbencoding.h"
#include "util.h"
#include "smtapi.h"

using namespace smtapi;

OMTSATPBEncoding::OMTSATPBEncoding(MRCPSP * instance) : MRCPSPEncoding(instance)  {
}

SMTFormula * OMTSATPBEncoding::encode(int lb, int ub){

	int N = ins->getNActivities();
	int N_RR = ins->getNRenewable();
	int N_NR = ins->getNNonRenewable();
	int N_R = ins->getNResources();

	SMTFormula * f = new SMTFormula();

	//Start time integer variables
	vector<intvar> S(N+1);
	for (int i=0;i<=N+1;i++)
		S[i]=f->newIntVar("S",i);

	//Activity 0 starts at time 0
	f->addClause(S[0] == 0);

	//Set bounds on makespan
	f->addClause(S[N+1] >= lb);
	f->addClause(S[N+1] <= ub);


	//Execution modes of the activities
	for (int i=0;i<=N+1;i++) {
		vector<literal> vmodes;
		for (int p=0;p<ins->getNModes(i);p++)
			vmodes.push_back(f->newBoolVar("sm",i,p));
		f->addEO(vmodes); //Each activity has exactly one execution mode
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


	//Definition of x_i,t,o
	for (int i=1;i<=N;i++) {
		for (int g=0;g<ins->getNModes(i);g++) {
			for (int t=ins->ES(i);t<ins->LC(i,ub);t++) {
				boolvar x = f->newBoolVar("x",i,t,g);
				boolvar sm = f->bvar("sm",i,g);
				literal geSi = S[i] <= t;
				literal ltCi = (t-ins->getDuration(i,g)) < S[i];

				f->addClause(!x | geSi);
				f->addClause(!x | ltCi);
				f->addClause(!x | sm);
				f->addClause(x | !geSi | !ltCi | !sm);
			}
		}
	}


	//Renewable resource constraints
	for (int r=0;r<N_RR;r++) {
		vector<int> lastvtasks;
		vector<set<int> > groups;

		for (int t=0;t<ub;t++) {
			vector<vector<literal> > vars_group;
			vector<vector<int> > coefs_group;
			vector<int> vtasks;

			for (int i=1;i<=N;i++)
				if (t>=ins->ES(i) && t<ins->LC(i,ub))
					vtasks.push_back(i);

			if(lastvtasks != vtasks && !vtasks.empty()){
				groups.clear();
				ins->computeMinPathCover(vtasks,groups);
			}


			for (const set<int> & group : groups) {
				vector<literal> vars_part;
				vector<int> coefs_part;

				for(int i : group){
					for (int g=0;g<ins->getNModes(i);g++) {
						int auxir=ins->getDemand(i,r,g);
						if (auxir!=0) {
							vars_part.push_back(f->bvar("x",i,t,g));
							coefs_part.push_back(auxir);
						}
					}
				}

				if(!coefs_part.empty()){
					vars_group.push_back(vars_part);
					coefs_group.push_back(coefs_part);
				}
			}

			util::sortCoefsDecreasing(coefs_group,vars_group);

			if (!vars_group.empty())
				f->addAMOPB(coefs_group,vars_group,ins->getCapacity(r),AMOPB_AMOMDD);
		}
	}


	//Non-renewable resource constraints
	for (int r=N_RR;r<N_R;r++) {
		vector<vector<literal> > vars_group;
		vector<vector<int> > coefs_group;
		for (int j=1;j<=N;j++) {
			vector<literal> vars_part;
			vector<int> coefs_part;
			for (int g=0;g<ins->getNModes(j);g++) {
				int d=ins->getDemand(j,r,g);
				if (d!=0) {
					vars_part.push_back(f->bvar("sm",j,g));
					coefs_part.push_back(d);
				}
			}
			if(!coefs_part.empty()){
				vars_group.push_back(vars_part);
				coefs_group.push_back(coefs_part);
			}
		}
		util::sortCoefsDecreasing(coefs_group,vars_group);

		if (!vars_group.empty())
			f->addAMOPB(coefs_group,vars_group,ins->getCapacity(r),AMOPB_AMOMDD);
	}

	f->minimize(S[N+1]);
	f->setLowerBound(lb); //Inclusive bounds. Turned to exclusive in formula generation time
	f->setUpperBound(ub);

	return f;
}

void OMTSATPBEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
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


OMTSATPBEncoding::~OMTSATPBEncoding() {
}
