#include "doubleorder.h"
#include "util.h"
#include <limits.h>

DoubleOrder::DoubleOrder(MRCPSP * instance, AMOPBEncoding amopbenc, bool maxsat) : MRCPSPEncoding(instance)  {
	this->maxsat = maxsat;
	this->amopbenc = amopbenc;
}


SMTFormula * DoubleOrder::encode(int lb, int ub){
   int N = ins->getNActivities();
   int N_RR = ins->getNRenewable();
   int N_NR = ins->getNNonRenewable();
	int N_R = ins->getNResources();

   SMTFormula * f = new SMTFormula();


	for(int i=0;i<N+2; i++){

      //DoubleOrder encoding of the start times
		int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);
      int ECi=ins->EC(i);
      int LCi=ins->LC(i,ub);
      if(ESi > LSi){
         f->addEmptyClause();
         return f;
      }
		for(int t=ESi; t <= LSi; t++){
			boolvar o = f->newBoolVar("o",i,t); //Create variable o_{i,t}
			if(t>ESi)
				f->addClause(!f->bvar("o",i,t-1) | o); // o_{i,t-i} -> o_{i,t}
		}
      f->addClause(f->bvar("o",i,LSi)); //It has started for sure at the latest start time
      for(int t=ECi; t <= LCi; t++){
			boolvar o2 = f->newBoolVar("o'",i,t); //Create variable o'_{i,t}
			if(t>ECi)
			f->addClause(!f->bvar("o'",i,t-1) | o2); // o'_{i,t-i} -> o'_{i,t}
		}
      f->addClause(f->bvar("o'",i,LCi)); //It has started for sure at the latest start time


      //Create variable sm_{i,o}
      vector<literal> vmodes;
      for (int p=0;p<ins->getNModes(i);p++)
			vmodes.push_back(f->newBoolVar("sm",i,p));

      //Exactly one execution mode
      f->addEO(vmodes);


      vector<int> ordmodes = ins->getModesOrdByDur(i);

      //Channeling between the two order encodings
      for(int t=ESi-1; t <= LSi+1; t++){
         boolvar o = t < ESi ? f->falseVar() : (t > LSi ? f->trueVar() : f->bvar("o",i,t));
         for (int m=0;m<ins->getNModes(i);m++){
            boolvar sm = f->bvar("sm",i,m);
            int t2 = t+ins->getDuration(i,m);
            boolvar o2 = t2 < ECi ? f->falseVar() : ( t2 > LCi ? f->trueVar() : f->bvar("o'",i,t2));

            f->addClause(!sm | !o |  o2);
            f->addClause(!sm |  o | !o2);
         }
         int tmin = t+ins->getMinDuration(i);
         int tmax = t+ins->getMaxDuration(i);
         boolvar o2min = tmin < ECi ? f->falseVar() : ( tmin > LCi ? f->trueVar() : f->bvar("o'",i,tmin));
         boolvar o2max = tmax < ECi ? f->falseVar() : ( tmax > LCi ? f->trueVar() : f->bvar("o'",i,tmax));

         //Minimum and maximum durations
         f->addClause( o | !o2min);
         f->addClause(!o |  o2max);

         //Domain reasoning
         for (int m=0;m<ins->getNModes(i)-2;m++){
            clause c = o;
            for(int m2 = 0; m2 <= m; m2++)
               c |= f->bvar("sm",i,ordmodes[m2]);
            int t2 = t+ins->getDuration(i,ordmodes[m+1]);
            c |= !(t2 < ECi ? f->falseVar() : ( t2 > LCi ? f->trueVar() : f->bvar("o'",i,t2)));
            f->addClause(c);
	}
         for (int m=ins->getNModes(i)-1;m>=2;m--){
            clause c = !o;
            for(int m2 = ins->getNModes(i)-1; m2 >= m; m2--)
               c |= f->bvar("sm",i,ordmodes[m2]);
            int t2 = t+ins->getDuration(i,ordmodes[m-1]);
            c |= t2 < ECi ? f->falseVar() : ( t2 > LCi ? f->trueVar() : f->bvar("o'",i,t2));
            f->addClause(c);
	}
      }

      //Create variable x_{i,t,o}
		for(int t=ESi; t < LCi; t++)
			for(int m = 0; m < ins->getNModes(i); m++)
				f->newBoolVar("x",i,t,m);


	}

   f->addClause(f->bvar("o",0,0)); //S_0 = 0
   if(lb>ins->ES(N+1))
      f->addClause(!f->bvar("o",N+1,lb-1));

	//Definition of x_{i,t,o}
	for(int i=0; i < N+2; i++){
		for(int m = 0; m < ins->getNModes(i); m++){
			for(int t=ins->ES(i); t < ins->LC(i,ub); t++){
				boolvar x = f->bvar("x",i,t,m);
				boolvar o = t <= ins->LS(i,ub) ? f->bvar("o",i,t) : f->trueVar();
				boolvar o2 = t >= ins->EC(i) ? f->bvar("o'",i,t) : f->falseVar();
				f->addClause(!f->bvar("sm",i,m) | !o | o2 | x );
				f->addClause(f->bvar("sm",i,m) | !x );
				f->addClause(o | !x );
				f->addClause(!o2| !x );
			}
		}
	}

	for(int i=0; i< N+2; i++){
		int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);
		int ECi=ins->EC(i);
		int LCi=ins->LC(i,ub);
		for(int j : ins->getSuccessors(i)){
			int ESj=ins->ES(j);
			int LSj=ins->LS(j,ub);

         //Precedences
         int mint = min(ECi,ESj);
         int maxt = max(LCi,LSj);
         for(int t=mint; t <= maxt; t++){
            boolvar o2i;
            boolvar oj;
            if(t < ECi)
               o2i = f->falseVar();
            else if(t > LCi)
               o2i = f->trueVar();
            else
               o2i=f->bvar("o'",i,t);   // i finishes at t or later
            if(t<ESj)
               oj=f->falseVar();
            else if(t > LSj)
               oj=f->trueVar();
            else
               oj=f->bvar("o",j,t);  //  j starts at t or earlier
            f->addClause(o2i | !oj);
         }
		}
	}

  //Renewable resource constraints
	for (int r=0;r<N_RR;r++) {
		vector<int> lastvtasques;
		vector<set<int> > groups;

		for (int t=0;t<ub;t++) {
			vector<vector<literal> > vars_group;
			vector<vector<int> > coefs_group;
			vector<int> vtasques;

			for (int i=1;i<=N;i++) {
				int ESi=ins->ES(i);
				int LCi=ins->LC(i,ub);
				if (t>=ESi && t<LCi)
					vtasques.push_back(i);
			}

			if(lastvtasques != vtasques && !vtasques.empty()){
				groups.clear();
				ins->computeMinPathCover(vtasques,groups);
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
				f->addAMOPB(coefs_group,vars_group,ins->getCapacity(r),amopbenc);
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
			f->addAMOPB(coefs_group,vars_group,ins->getCapacity(r),amopbenc);
	}

	if(maxsat)
		for(int t=ins->ES(N+1); t <= ins->LS(N+1,ub); t++)
			f->addSoftClause(f->bvar("o",N+1,t));

   return f;
}

void DoubleOrder::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();

	this->modes=vector<int>(N+2);
   this->starts=vector<int>(N+2);
	this->modes[0] = 0;
	this->modes[N+1] = 0;

	for (int i=0;i<=N+1;i++) {
		for (int p=0;p<ins->getNModes(i);p++){
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel)){
				this->modes[i]=p;
            break;
         }
		}

      for(int t = ins->ES(i); t <= ins->LS(i,ub); t++){
         if(SMTFormula::getBValue(ef.f->bvar("o",i,t),bmodel)){
            this->starts[i]=t;
            break;
         }
      }
	}
}

bool DoubleOrder::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->bvar("o",N+1,ub));
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				for(int g = 0; g < ins->getNModes(i); g++)
					ef.f->addClause(!ef.f->bvar("x",i,t,g));

		return true;
	}
	else return false;
}

DoubleOrder::~DoubleOrder() {
}
