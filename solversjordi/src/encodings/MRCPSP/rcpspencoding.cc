#include "MRCPSP/rcpspencoding.h"
#include <limits.h>
#include <assert.h>

RCPSPEncoding::RCPSPEncoding(MRCPSP * instance, AMOPBEncoding amopbenc) : MRCPSPEncoding(instance)  {
	this->amopbenc = amopbenc;
	this->usepb = false;
	this->printpb = false;
	this->printamo = false;
}

void RCPSPEncoding::setPrintPB(const string & filename){
	this->printpb = true;
	this->pbfilename = filename;
}

void RCPSPEncoding::setPrintAMO(const string & filename){
	this->printamo = true;
	this->amofilename = filename;
}

void RCPSPEncoding::setUsePB(PBEncoding pbenc){
	this->usepb = true;
	this->pbenc = pbenc;
} 

 SMTFormula * RCPSPEncoding::encode(int lb, int ub){
    int N = ins->getNActivities();
    int N_RR = ins->getNRenewable();
    
    SMTFormula * f = new SMTFormula();
    
    vector<vector<literal> > x (N+1);
    vector<vector<literal> > s (N+2);

    std::vector<float> dummy_features;

	//Creation of x_i,t and s_i,t
	for (int i=1;i<N+1;i++) {
        x[i].resize(ub);
        for (int t = ins->ES(i); t < ins->LC(i, ub); t++) {
            x[i][t]= f->newBoolVar("x",i,t,dummy_features);
        }
    }
    for (int i=0;i<N+2;i++) {
        s[i].resize(ub+1);
        for (int t = ins->ES(i); t <= ins->LS(i, ub); t++) {
            s[i][t]= f->newBoolVar("s",i,t,dummy_features);
        }
    }
    
    //Order encoding of the start times
	for(int i=0;i<N+2; i++){
        int ESi=ins->ES(i);
        int LSi=ins->LS(i,ub);
        
        if(ESi>LSi) {
			std::vector<float> vfl;
			vfl.push_back(0); // La variable és d'inicis
			vfl.push_back(0); // La variable no és de execucions
            f->addClause(f->falseVar(&vfl));
			return f;
		}
        
        if(LSi>0)
            f->addClause(f->bvar("s",i,LSi));
        for(int t=ESi; t < LSi; t++)
            f->addClause(!f->bvar("s",i,t) | f->bvar("s",i,t+1));
	}

	//Definition of x_{i,t}
	for(int i=1; i < N+1; i++){
        int ESi=ins->ES(i);
        int LSi=ins->LS(i,ub);
        int ECi=ins->EC(i);
        int LCi=ins->LC(i,ub);
        int dur = ins->getDuration(i,0);

        for(int t=ESi; t < LCi; t++){
            if(t<=LSi && t-dur>=ESi)
                f->addClause(f->bvar("x",i,t) | !f->bvar("s",i,t) | f->bvar("s",i,t-dur));
            else if(!(t<=LSi) && t-dur>=ESi)
                f->addClause(f->bvar("x",i,t) | f->bvar("s",i,t-dur));
            else if(t<=LSi && !(t-dur>=ESi))
                f->addClause(f->bvar("x",i,t) | !f->bvar("s",i,t));
            else
                f->addClause(f->bvar("x",i,t));

            
        }

	}

	//Precedences
	for(int i=1; i< N+2; i++){
		int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);
        int dur = ins->getDuration(i,0);
        
		for(int j : ins->getSuccessors(i)){
			int ESj=ins->ES(j);
			int LSj=ins->LS(j,ub);

            int mint = min(ESi,ESj-dur);
            int maxt = max(LSj-dur,LSi);
            for(int t=mint; t <= maxt; t++){
                boolvar oi;
                boolvar oj;
                if(t < ESi)
                    oi = f->falseVar(&dummy_features);
                else if(t > LSi)
                    oi = f->trueVar(&dummy_features);
                else
                    oi=f->bvar("s",i,t);   // i starts at t or earlier
                if(t+dur<ESj)
                    oj=f->falseVar(&dummy_features);
                else if(t +dur > LSj)
                    oj=f->trueVar(&dummy_features);
                else
                    oj=f->bvar("s",j,t+dur);  //  j starts at t plus dur or earlier
                f->addClause( oi | !oj);
            }
		}
	}
    
    //Renewable resource constraints
	for (int r=0;r<N_RR;r++) {
		for (int t=0;t<ub;t++) {
            char aux[50];
            sprintf(aux,"R_%d_%d",r,t);
            f->setAuxBoolvarPref(aux);

			vector<vector<literal> > X;
			vector<vector<int> > Q;

            // Activitats que es poden executar en aquest temps
            vector<int> vtasques;
            vector<set<int> > groups;

            for (int i=1;i<=N;i++) {
                int ESi=ins->ES(i);
                int LCi=ins->LC(i,ub);
                if (t>=ESi && t<LCi)
                    vtasques.push_back(i);
            }

            if(!vtasques.empty())
                ins->computeMinPathCover(vtasques,groups);

            for (const set<int> & group : groups) {
                //X.push();
                //Q.push();
                vector<literal> vars_part;
                vector<int> coefs_part;
				bool printed = false;
                for(int i : group){
                    int auxir=ins->getDemand(i,r,0);
                    if (auxir!=0) {
                        boolvar x = f->bvar("x",i,t);
                        vars_part.push_back(x);
                        coefs_part.push_back(auxir);
                    }
                }

                if(!coefs_part.empty()){
                    X.push_back(vars_part);
                    Q.push_back(coefs_part);
                }
            }

			if (!X.empty()){
				util::sortCoefsDecreasing(Q,X);

                f->addAMOPB(Q,X,ins->getCapacity(r),amopbenc,&dummy_features);
            }
		}
	}

    // Extended Precedences
	for (int i=0;i < N+2;i++) {
		for (int j=0;j < N+2;j++) { 
			int extprec = ins->getExtPrec(i,j);
			int maxt = std::min(ins->LS(j,ub), ins->LS(i, ub));
			if(extprec>0) {
				for (int t = ins->ES(j); t < maxt; t++) {
					if (t-extprec >= ins->ES(i))
						f->addClause(!f->bvar("s",j,t) | f->bvar("s",i,t-extprec));
					else
						f->addClause(!f->bvar("s",j,t));
				}
			}
		}
	}

    return f;
}

void RCPSPEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();

	this->modes=vector<int>(N+2);
    this->starts=vector<int>(N+2);
	this->modes[0] = 0;
	this->modes[N+1] = 0;

	for (int i=0;i<=N+1;i++) {

        int ESi=ins->ES(i);
	    int LSi=ins->LS(i,ub);

        for(int t = ESi; t <= LSi; t++){
            if(SMTFormula::getBValue(ef.f->bvar("s",i,t),bmodel)){
                this->starts[i]=t;
                break;
            }
        }
	}
}

bool RCPSPEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
    
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->bvar("s",N+1,ub));
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				ef.f->addClause(!ef.f->bvar("x",i,t));

		return true;
	}
	else return false;
}

RCPSPEncoding::~RCPSPEncoding() {

}
