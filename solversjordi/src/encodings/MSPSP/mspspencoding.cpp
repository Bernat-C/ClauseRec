#include "mspspencoding.h"
#include <set>
#include "util.h"

using namespace smtapi;

MSPSPEncoding::MSPSPEncoding(MSPSP * instance) : Encoding() {
	this->ins = instance;
}

MSPSPEncoding::~MSPSPEncoding() {
}


void MSPSPEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}

int MSPSPEncoding::getObjective() const{
	return starts.back();
}

bool MSPSPEncoding::printSolution(ostream & os) const{
	ins->printSolution(os,starts, assignment);
	
	return true;
}


void MSPSPEncoding::assertNBDD(NBDD * nb, const vector<literal> & selectors, SMTFormula * f) const{
	if(nb->isLeafNBDD()){
		if(nb->isFalseNBDD())
			f->addEmptyClause();
		return;
	}

	int N = nb->getIdBasedSize();

	vector<boolvar> nodevar(N);
	vector<boolvar> trueedgevar(N);
	vector<boolvar> falseedgevar(N);

	vector<vector<NBDD *> > nodesOfLayer(selectors.size()+1); //Nodes of each layer
	vector<bool> visited(N,false);
	nb->getNodesByLayer(visited,nodesOfLayer,0);

	vector<vector<literal> > inputedges(N);

	for(int i = 2; i < N-1; i++){
		nodevar[i] = f->newBoolVar();
		trueedgevar[i] = f->newBoolVar();
		falseedgevar[i] = f->newBoolVar();
	}
	//The root also needs edge variables
	trueedgevar[N-1] = f->newBoolVar();
	falseedgevar[N-1] = f->newBoolVar();

	//The root must be true 
	nodevar[N-1]=f->trueVar();
	
	//The true terminal node must be true
	nodevar[1]=f->trueVar();

	//The false terminal node will have a dummy auxilliary variable
	nodevar[0]=f->newBoolVar();

	
	//For all layers except the terminal
	for(int layer = 0; layer < selectors.size(); layer++){
		clause c3T=!selectors[layer];
		clause c3F=selectors[layer];
		clause c4;
		for(NBDD * nb : nodesOfLayer[layer]){ //Forall nodes (terminals already excluded)
			int i = nb->getId();
			//T1: if the node is true, one of its output edges must be true
			f->addClause(!nodevar[i] | trueedgevar[i] | falseedgevar[i]);

			//T2: if the edge is true, the source node is true
			f->addClause(!trueedgevar[i] | nodevar[i]);
			f->addClause(!falseedgevar[i] | nodevar[i]);

			//T3: if the edge is true, the destination node is true
			for(NBDD * child : nb->getTrueChildren()){
				f->addClause(!trueedgevar[i] | nodevar[child->getId()]);
				inputedges[child->getId()].push_back(trueedgevar[i]);
			}
			f->addClause(!falseedgevar[i] | nodevar[nb->getFalseChild()->getId()]);
			inputedges[nb->getFalseChild()->getId()].push_back(falseedgevar[i]);

			//T4: if the edge is true, the selector literal is true
			f->addClause(!trueedgevar[i] | selectors[layer]);
			f->addClause(!falseedgevar[i] | !selectors[layer]);

			//P1: if the source node and the selector are true, the edge is true
			f->addClause(!nodevar[i] | !selectors[layer] | trueedgevar[i]);
			f->addClause(!nodevar[i] | selectors[layer] | falseedgevar[i]);


			if(!nb->getTrueChildren()[0]->isFalseNBDD())
				c3T|=trueedgevar[i];
			if(!nb->getFalseChild()->isFalseNBDD())
				c3F|=falseedgevar[i];

			c4 |= nodevar[nb->getId()];
		}

		//P3: if a selector is true (false), at least one associated edge (not pointing to F-terminal) must be true
		f->addClause(c3T);
		f->addClause(c3F);

		//P4: at least one node of each layer is true
		f->addClause(c4);
		
	}

	//Forall nodes except the root
	for(int i = 0; i < N-1; i++)
		//P2: if the node is true, one of its input edges must be true
		f->addClause(!nodevar[i] | inputedges[i]);

}
