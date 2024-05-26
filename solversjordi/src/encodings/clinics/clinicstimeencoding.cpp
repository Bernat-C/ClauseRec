#include "clinicstimeencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

ClinicsTimeEncoding::ClinicsTimeEncoding(Clinics * instance) : ClinicsEncoding(instance)  {
}

ClinicsTimeEncoding::~ClinicsTimeEncoding() {

}

SMTFormula * ClinicsTimeEncoding::encode(int lb, int ub){

	SMTFormula * f = new SMTFormula();
	int n = ins->getNActivities();
	vector<intvar> S(n+2);

	//Create start time variables and set time windows
	for(int i = 0; i < n+2; i++){
		S[i] = f->newIntVar("S",i);
		f->addClause(S[i] >= ins->ES(i));
		f->addClause(S[i] <= ins->LS(i,ub));
	}

	//Start at time 0
	f->addClause(S[0]==0);

	//Bound objective function
	f->addClause(S[n+1]>=lb);
	f->addClause(S[n+1]<=ub);

	//Add precedences
	for(int i = 0; i < n+2; i++)
		for(int j = 0; j < n+2; j++)
			if(i!=j && ins->getExtPrec(i,j)!=INT_MIN)
				f->addClause(S[j]-S[i]>=ins->getExtPrec(i,j));

	//Add mutual exclucions
	for(int i = 1; i <= n-1; i++){
		for(int j = i+1; j <= n; j++){
			if(ins->needMutex(i,j)){
				int duri, durj;
				if(ins->needClean(i,j)){
					duri=ins->getCleanDuration(i);
					durj=ins->getCleanDuration(j);
				}
				else{
					duri=ins->getDuration(i);
					durj=ins->getDuration(j);
				}
				f->addClause(S[i] - S[j] >= durj | S[j] - S[i] >= duri);
			}
		}
	}

	//Add resource constraints
	for(int r = 0; r < ins->getNResources(); r++){
		for(int t = 0; t < ub; t++){
			vector<literal> lits;
			int d = 0;
			for(const pair<int,int> & p : ins->getDemands(r)){
				int s = p.first; //Start
				int e = p.second; //End
				if(ins->ES(s)<=t && t<ins->LC(e,ub)){
					//x <-> Ss <= t & t < Se + dur(e)
					boolvar x = f->newBoolVar("x",r,d,t);
					literal l1 = S[s] <= t;
					literal l2 = t - ins->getDuration(e) < S[e];
					f->addClause(!x | l1);
					f->addClause(!x | l2);
					f->addClause(x | !l1 | !l2);
					lits.push_back(x);
				}
				d++;
			}
			if(!lits.empty())
				f->addAMK(lits,ins->getCapacity(r));
		}
	}

	return f;
}

bool ClinicsTimeEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
    if(lastLB <= lb && lastUB >= ub){
		int n = ins->getNActivities();
		ef.f->addClause(ef.f->ivar("S",n+1)>=lb);
		ef.f->addClause(ef.f->ivar("S",n+1)<=ub);
		for(int r = 0; r < ins->getNResources(); r++){
			for(int t = ub; t < lastUB; t++){
				int d = 0;
				for(const pair<int,int> & p : ins->getDemands(r)){
					int s = p.first; //Start
					int e = p.second; //End
					if(ins->ES(s)<=t && t<ins->LC(e,ub))
						ef.f->addClause(!ef.f->bvar("x",r,d,t));
					d++;
				}
			}
		}
		return true;
    }
    else return false;
}
