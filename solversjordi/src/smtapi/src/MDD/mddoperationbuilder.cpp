#include "MDDOperationBuilder.h"
#include "util.h"



MDDOperationBuilder::MDDOperationBuilder(MDD * m1, MDD * m2, int op)
	: MDDBuilder(){

	this->m1 = m1;
	this->m2 = m2;
	this->op = op;

	H = new map<vector<MDD *>,MDD *>[max(m1->getVarDepth(),m2->getVarDepth())+1];
	nodeCount = 2;

}

MDDOperationBuilder::~MDDOperationBuilder(){
  delete [] H;
}

MDD *  MDDOperationBuilder::buildMDD(){
	return apply(m1,m2);
}

MDD * MDDOperationBuilder::apply(MDD * m1, MDD * m2){
	map<pair<int,int>,MDD *>::iterator it = G.find(pair<int,int>(m1->getId(),m2->getId()));
	if(it!=G.end()) return it->second;
	else{
		MDD * m;
		vector<MDD *> children;
		if(m1->isLeafMDD() && m2->isLeafMDD())
			m = applyToLeaf(m1,m2);
		else if(m1->getVarDepth() == m2->getVarDepth()){
			for(int i = 0; i < m1->getNSelectors(); i++)
				children.push_back(apply(m1->getChildByIdx(i),m2->getChildByIdx(i)));
			children.push_back(apply(m1->getElseChild(),m2->getElseChild()));
			m=mk(m1->getSelectors(), children, m1->getVarDepth());
		}
		else if(m1->getVarDepth() > m2->getVarDepth()) {
			for(int i = 0; i < m1->getNSelectors(); i++)
				children.push_back(apply(m1->getChildByIdx(i),m2));
			children.push_back(apply(m1->getElseChild(),m2));
			m=mk(m1->getSelectors(), children, m1->getVarDepth());
		}
		else{
			for(int i = 0; i < m2->getNSelectors(); i++)
				children.push_back(apply(m1,m2->getChildByIdx(i)));
			children.push_back(apply(m1,m2->getElseChild()));
			m=mk(m2->getSelectors(), children, m2->getVarDepth());
		}
		G[pair<int,int>(m1->getId(),m2->getId())]=m;
		return m;
	}
}

MDD * MDDOperationBuilder::applyToLeaf(MDD * m1, MDD * m2){
	if(m1->isTrueMDD()){
		if(m2->isTrueMDD()) return MDD::MDDTrue();
		else return op==OR ? MDD::MDDTrue() : MDD::MDDFalse();
	}
	else{
		if(m2->isTrueMDD()) return op == OR ? MDD::MDDTrue() : MDD::MDDFalse();
		else return MDD::MDDFalse();
	}
}

MDD * MDDOperationBuilder::mk(const vector<boolvar> & selectors, const vector<MDD *> & children, int depth){
	bool equals = true;
	MDD * m = children[0];
	int i = 1;
	while(i < children.size() && equals){
		equals = children[i]==m;
		i++;
	}

	if(equals) return m;
	else{
		map<vector<MDD *>,MDD *>::iterator it = H[depth].find(children);
		if(it != H[depth].end())
			return it->second;
		else{
			m = new MDD(nodeCount++,depth-1);
			for(i = 0; i < selectors.size(); i++)
				m->addChild(selectors[i],children[i]);
			m->setElseChild(children[i]);
			H[depth][children]=m;
			return m;
		}
	}
}


void MDDOperationBuilder::createGraphviz(ostream & os, vector<vector<int> > * labels) const{
  	if(root == NULL)
		return;

  bool * visited = new bool [root->getId()+1];
  for(int i = 0; i < root->getId()+1; i++)
	  visited[i]=false;

  os << "digraph G {" << endl;

   const vector<vector<int> > & lab = *labels;

  //Comment full expressions
// //   os << "#";
// //    for(int i = 0; i < lab.size()-1; i++){
// //       os << "{" << lab[i][0];
// //       for(int j = 1; j < lab[i].size(); j++)
// // 	os << "," << lab[i][j];
// //       os << "},";
// //     }
// //     os << "{" << lab[lab.size()-1][0];
// //     for(int j = 1; j < lab[lab.size()-1].size(); j++)
// //       os << "," << lab[lab.size()-1][j];
// //     os << "} <= " << K  << endl;


  createGraphviz(root,os,visited,labels);

  for(int i = 0; i < lab.size(); i++){
	  os << "{rank=same";
	  for(const pair<vector<MDD *>,MDD*> & p : H[i])
		  os << " n_" << p.second->getId();

	  os << "}" << endl;
  }
  os << "{rank=same n_" << MDD::MDDTrue()->getId() << " n_" << MDD::MDDFalse()->getId() << "}" << endl;

  os << "}" << endl;
  delete [] visited;
}


void MDDOperationBuilder::createGraphviz(MDD * mdd, ostream & os, bool * visited, vector<vector<int> > * labels) const{
	const vector<vector<int> > & lab = *labels;
	visited[mdd->getId()]=true;
	if(mdd->isTrueMDD())
		os << "n_" << mdd->getId() << "[label=\"1\",shape=box];" << endl;
	else if(mdd->isFalseMDD())
		os << "n_" << mdd->getId() << "[label=\"0\",shape=box];" << endl;
	else{
		os << "n_" << mdd->getId() << "[label=\"";
		int l = lab.size() + 1  - mdd->getVarDepth();
		os << "{" << lab[l][0];
		for(int j = 1; j < lab[l].size(); j++)
			os << "," << lab[l][j];
		os << "}";
		if(l < lab.size()-1)
			os << "+[...]";
		//os << " <= [" << B << "," << Y << "]\"];" << endl;
		os << "\"];" << endl;


		for(int i = 0; i < mdd->getNSelectors(); i++){
			MDD * child = mdd->getChildByIdx(i);
			if(!visited[child->getId()])
				createGraphviz(child,os,visited,labels);
			os << "n_" << mdd->getId() << " -> n_" << child->getId() << " [label=\"" << lab[l][i] << "\"];" << endl;
		}
		MDD * child = mdd->getElseChild();
		if(!visited[child->getId()])
			createGraphviz(child,os,visited,labels);
		os << "n_" << mdd->getId() << " -> n_" << child->getId() << " [label=\"else\"];" << endl;
	}
}
