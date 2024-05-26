#include "auctionencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

AuctionEncoding::AuctionEncoding(Auction * instance, AMOPBEncoding amopbenc) : Encoding()  {
	this->instance = instance;
	this->amopbenc = amopbenc;
}

AuctionEncoding::~AuctionEncoding() {

}

SMTFormula * AuctionEncoding::encode(int lb, int ub){

	SMTFormula * f = new SMTFormula();

	vector<literal> x(instance->getNBids());

    for(int i = 0; i < instance->getNBids(); i++)
		x[i]=f->newBoolVar("x",i);

    for(int i = 0; i < instance->getNBids()-1; i++)
		for(int item : instance->getBid(i))
			for(int j = i+1; j < instance->getNBids(); j++)
				if(instance->demandsItem(j,item))
					f->addClause(!x[i] | !x[j]);

	vector<vector<int> > cover;
	instance->computeBidCover(cover);

	vector<vector<literal> > X(cover.size());
	vector<vector<int> > Q(cover.size());

	for(int i = 0; i < cover.size(); i++){
		for(int j : cover[i]){
			Q[i].push_back(instance->getBidValue(j));
			X[i].push_back(x[j]);
		}
	}

	int nVars = f->getNBoolVars();
	int nClauses = f->getNClauses();

	f->addAMOPBGEQ(Q,X,lb,amopbenc);

	// boolvar -> literal -> clause
	//boolvar v;
	//clause c;
	//for(int i = 0; i < 10; i++)
	//	c|= x[i];
	//clause c = !v;
	//f->addClause(v);
	//f->addClause(!v);
	//f->addClause(v | !x | z);
	//f->addEO(X);
	//f->addAMO(X);
	//f->addALO(X);
	//f->addALK(X,K);
	//f->addAMK(X,K);
	//f->addEK(X,K);
	//f->addALK(X,K);
	//f->addALK(X,K);
	//f->addPB(X,Q,K);

	nVars = f->getNBoolVars() - nVars;
	nClauses = f->getNClauses() - nClauses;

	cerr << "c amopb;" << nVars << ";" << nClauses << endl;

	exit(0);

	return f;
}


void AuctionEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

	this->sold.resize(instance->getNBids());
	for (int i=0; i<instance->getNBids(); i++)
		sold[i] = SMTFormula::getBValue(ef.f->bvar("x",i),bmodel);
}

int AuctionEncoding::getObjective() const{
	int sum = 0;
	for (int i=0; i<instance->getNBids(); i++)
		if(sold[i])
			sum+=instance->getBidValue(i);

	return sum;
}


bool AuctionEncoding::printSolution(ostream & os) const {
	for(int i = 0; i < instance->getNBids(); i++)
		if(sold[i])
			os << " Sold:" << i;
	os << endl;
	return true;
}
