#include "nbdd.h"
#include "util.h"
#include <map>
#include "mspsp.h"

NBDD::NBDD(int id,int nvars) {
	this->id = id;
	istrivialtrue=false;
	istrivialfalse=false;
	vardepth=nvars+1;
	realdepth = 0;
	recursivedelete = true;
}

NBDD::NBDD(bool b){
	id = b ? 1 : 0;
	istrivialtrue = b;
	istrivialfalse = !b;
	vardepth = 1;
	realdepth = 1;
	recursivedelete = true;
}

NBDD::~NBDD(){
	if(recursivedelete && !isLeafNBDD()){
		vector<NBDD *> children;
		for(NBDD * child : truechildren)
			child->unmarkRecursiveDelete(children);
		falsechild->unmarkRecursiveDelete(children);
		for(NBDD * n : children)
			delete n;
	}
}

void NBDD::unmarkRecursiveDelete(vector<NBDD *> & children){
	if(recursivedelete){
		recursivedelete=false;
		children.push_back(this);
		if(!isLeafNBDD()){
			for(NBDD * child : truechildren)
				child->unmarkRecursiveDelete(children);
			falsechild->unmarkRecursiveDelete(children);
		}
	}
}

const vector<NBDD *> & NBDD::getTrueChildren() const{
	return truechildren;
}

NBDD * NBDD::getFalseChild() const{
	return falsechild;
}

int NBDD::getId() const{
	return id;
}

int NBDD::getVarDepth() const{
	return vardepth;
}

int NBDD::getRealDepth() const{
	return realdepth;
}

int NBDD::getSize() const{
	if(isLeafNBDD()) return 1;

	bool * visited = new bool[id+1];
	for(int i = 0; i < id+1; i++)
		visited[i]=false;
	int size = 0;
	getSize(visited,size);
	delete [] visited;
	return size;
}

void NBDD::getSize(bool * visited, int & size) const{
	if(!visited[id]){
		visited[id]=true;
		size+=1;
		if(!isLeafNBDD()){
			for(NBDD * child : truechildren)
				child->getSize(visited, size);
			falsechild->getSize(visited, size);
		}
	}
}

int NBDD::getIdBasedSize() const{
	return isLeafNBDD() ? 1 : id+1;
}

bool NBDD::isLeafNBDD() const{
	return istrivialtrue || istrivialfalse;
}

bool NBDD::isTrueNBDD() const{
	return istrivialtrue;
}

bool NBDD::isFalseNBDD() const{
	return istrivialfalse;
}

void NBDD::addTrueChild(NBDD * child){
	truechildren.push_back(child);
	if(child->realdepth >= this->realdepth)
		this->realdepth = child->realdepth+1;
}

void NBDD::setFalseChild(NBDD * child){
	falsechild = child;
	if(child->realdepth >= this->realdepth)
		this->realdepth = child->realdepth+1;
}

NBDD * NBDD::constructNBDD(MSPSP * instance, const vector<int> & resources, const vector<int> & goal) {
	int sumgoal = util::sum(goal);

	if(resources.empty())
		return new NBDD(sumgoal==0 ? true : false);

	vector<map<vector<int>,NBDD*> > L(resources.size());
	int layer=0;
	int idCount = 2;
	vector<int> occupancy(instance->getNSkills(),0);

	map<pair<set<NBDD*>,NBDD*>,NBDD *> H;

	NBDD * nbtrue = new NBDD(true);
	nbtrue->occupancies.push_back(goal);
	NBDD * nbfalse = new NBDD(false);

	NBDD * nb = constructNBDD(instance, resources, idCount, L,layer,H,occupancy,goal,sumgoal,nbtrue,nbfalse);

	if(nb == nbfalse)
		delete nbtrue;
	return nb;
}


NBDD * NBDD::constructNBDD(MSPSP * instance, const vector<int> & resources, int & idCount,
	vector<map<vector<int>,NBDD*> > & L, int layer, map<pair<set<NBDD*>,NBDD*>,NBDD *> & H,
	vector<int> & occupancy, const vector<int> & goal, int sumgoal,
	NBDD * nbtrue, NBDD * nbfalse) {

	//=========If the leaves are reached, check if correct==========
	if(layer==resources.size()){
		if(occupancy==goal)
			return nbtrue;
		else
			return nbfalse;
	}

	//=========Shortcuts to false==========
	int nassigned = util::sum(occupancy);
	//If there is no enough resources to fulfill the requests, return false
	if(sumgoal - nassigned > resources.size()-layer)
		return nbfalse;

	//If we exceed the demand of some skill, return false
	for(int l = 0; l < instance->getNSkills(); l++)
		if(occupancy[l]>goal[l])
			return nbfalse;


	//Check if some skill is not enough?


	//======The NBDD is not false due to any of the previous reasons========
	NBDD *nbdd_new=NULL;
	map<vector<int>,NBDD*>::iterator it = L[layer].find(occupancy);
    
    
    
    
	//If we already found an NBDD representing 'occupancy' at layer 'layer', finished
	if(it!=L[layer].end())
		nbdd_new=it->second;
	//Otherwise, construct the (new?) NBDD
	else{
		//Compute all children associated with a skill asignment. Update children set of occupancies
        set<NBDD *> truechildren;
        
        //Remove
        bool alltruechildrenToFalse = true;
        
		for(int l = 0; l < instance->getNSkills(); l++)
		if(instance->hasSkill(resources[layer],l)){
			occupancy[l]++;
			NBDD * child = constructNBDD(instance,resources, idCount,L,layer+1,H,occupancy,goal,sumgoal,nbtrue,nbfalse);
			//UNCOMMENTif(child != nbfalse)
				truechildren.insert(child);
            
            //remove
            alltruechildrenToFalse = alltruechildrenToFalse && child == nbfalse;
			occupancy[l]--;
		}

        /*UNCOMMENT bool alltruechildrenToFalse = truechildren.empty();
		if(alltruechildrenToFalse)
			truechildren.insert(nbfalse);*/
        

		//Compute false child
		NBDD * falsechild = constructNBDD(instance,resources, idCount,L,layer+1,H,occupancy,goal,sumgoal,nbtrue,nbfalse);

		//If all children point to false, return false
		if(alltruechildrenToFalse && falsechild==nbfalse)
			nbdd_new = nbfalse;

		else{
			//UNCOMMENT map<pair<set<NBDD*>,NBDD*>,NBDD *>::iterator it = H.find(pair<set<NBDD*>,NBDD*>(truechildren,falsechild));

			//If needed to construct new NBDD
			//UNCOMMENT if(it == H.end()){
				nbdd_new=new NBDD(idCount, resources.size() - layer);
				idCount++;
				for(NBDD * child : truechildren)
					nbdd_new->addTrueChild(child);
				nbdd_new->setFalseChild(falsechild);
				H[pair<set<NBDD*>,NBDD*>(truechildren,falsechild)]=nbdd_new;
			/*UNCOMMENT}
			else
				nbdd_new = it->second;
             */
		}
		L[layer][occupancy]=nbdd_new;
		nbdd_new->occupancies.push_back(occupancy);
	}
	return nbdd_new;
}

void NBDD::getNodesByLayer(vector<bool> & visited, vector<vector<NBDD*> > & layers, int layer){
	if(!visited[id]){
		visited[id] = true;
		if((isFalseNBDD() || isTrueNBDD())){
			layers[layers.size()-1].push_back(this);
		}
		else{
			layers[layer].push_back(this);
			for(NBDD * nb : truechildren)
				nb->getNodesByLayer(visited,layers,layer+1);
			falsechild->getNodesByLayer(visited,layers,layer+1);
		}
	}

}

void printt(const vector<vector<NBDD*> > & layers){
	int i = 0;
	for(const vector<NBDD *> v : layers){
		cerr << i << ": ";
		for(NBDD * nb : v){
			cerr << nb->getId() << " ";
		}
		cerr << endl;
		i++;
	}
}

bool NBDD::getAssignment(MSPSP * instance, const vector<int> & resources, const vector<int> & partial_assingment, vector<pair<int,int> > & assignment){
	assignment.clear();
	if(this->isLeafNBDD())
		return true;

	vector<bool> visited(getIdBasedSize(),false);
	vector<vector<NBDD*> > layers(resources.size()+1);
	vector<vector<int> > adj (getIdBasedSize(),vector<int>(getIdBasedSize(),-1));
	getNodesByLayer(visited,layers,0);

	//Fill adjacency matrix taking into account the partial assignment:
	//	0-false edge, 1-true edge, -1 disconnected (or assigned opposite) (remove false node)
	for(int layer = 0; layer < resources.size(); layer++){
		for(NBDD * nb : layers[layer]){
			if(partial_assingment[layer]!=0)
				for(NBDD * child : nb->getTrueChildren())
					if(!child->isFalseNBDD())
						adj[nb->getId()][child->getId()]=1;
			if(partial_assingment[layer]!=1)
				if(!nb->getFalseChild()->isFalseNBDD())
					adj[nb->getId()][nb->getFalseChild()->getId()]=0;
		}
	}
	//Remove false node
	if(layers[resources.size()][0]->isFalseNBDD())
		layers[resources.size()][0] = layers[resources.size()][1];
	layers[resources.size()].resize(1);

	//Remove unreachable and deadend nodes until fix point
	bool changed;
	do{
		changed=false;
		for(int layer = 0; layer <= resources.size() ; layer++){
			for(int i = layers[layer].size()-1; i >=0; i--){
				NBDD * nb = layers[layer][i];
				//Check if it has any incoming edge
				bool reachable = true;
				if(layer > 0){
					reachable = false;
					for(NBDD * above : layers[layer-1]){
						if(adj[above->getId()][nb->getId()] != -1){
							reachable=true;
							break;
						}
					}
				}
				bool continues=true;
				//Check if it has any outgoing edge
				if(reachable && layer < resources.size()){
					continues = false;
					for(NBDD * below : layers[layer+1]){
						if(adj[nb->getId()][below->getId()] != -1){
							continues=true;
							break;
						}
					}
				}
				if(!reachable || !continues){
					if(!reachable && layer < resources.size())
						for(NBDD * below : layers[layer+1])
							adj[nb->getId()][below->getId()] = -1;

					else if(!continues && layer > 0)
						for(NBDD * above : layers[layer-1])
							adj[above->getId()][nb->getId()] = -1;

					layers[layer].erase(layers[layer].begin()+i);
					changed = true;
					if(layers[layer].empty())
						return false; //If we cannot pass accross some layer, return false
				}
			}
		}
	}while(changed);

	//traverse the NBDD following the remaining edges and deciding an assigned skill at each step
	vector<int> occupancy = occupancies[0];
	NBDD * focus = this;
	for(int k = 0; k < resources.size(); k++){
		for(NBDD * next : layers[k+1]){
			int edge = adj[focus->getId()][next->getId()];
			if(edge != -1){
				bool found = false;
				//We have found an edge to continue the path. Check if an assignment is needed
				if(edge==1){
				//If the skill is used, decide a skill and add an assignment
					for(int i = 0; i < next->occupancies.size() && !found; i++){
						for(int l = 0; l < instance->getNSkills(); l++)
						if(instance->hasSkill(resources[k],l)){
							occupancy[l]++;
							if(occupancy==next->occupancies[i]){
								//We have found the skill
								assignment.push_back(pair<int,int>(resources[k],l));
								found = true;
								break;
							}
							occupancy[l]--;
						}
					}
				}

				focus=next;
				break;
			}
		}
	}
	return true;
}


void NBDD::createGraphviz(MSPSP * instance,  const vector<int> & resources, const vector<int> & goal, ostream & os){

	os << "digraph G {" << endl;

	if(this->isLeafNBDD())
		this->createNodeGraphviz(goal,-1,os);
	else{
		vector<vector<NBDD*> > layers(resources.size()+1);
		vector<bool> visited(id+1,false);
		getNodesByLayer(visited,layers,0);

		for(int i = 0; i < layers.size(); i++){
			int resource = i < layers.size()-1 ? resources[i] : -1;
			for(int j = 0; j < layers[i].size(); j++)
				layers[i][j]->createNodeGraphviz(goal,resource,os);
		}

		for(int i = 0; i < layers.size(); i++){
			os << "{rank=same";
			for(int j = 0; j < layers[i].size(); j++)
				os << " n_" << layers[i][j]->getId();
			os << "}" << endl;
		}
	}
	os << "}" << endl;
}


void NBDD::createNodeGraphviz(const vector<int> & goal, int resource, ostream & os){
	if(isTrueNBDD()){
		os << "n_" << id << "[label=\"T:(";
		bool comma=false;
		for(int i : goal){
			if(comma)
				os << ",";
			else
				comma=true;
			os << i;
		}
		os << ")\",shape=box];" << endl;
	}
	else if(isFalseNBDD())
		os << "n_" << id << "[label=\"F\",shape=box];" << endl;
	else{
		bool comma = false;
		os << "n_" << id << "[label=\"";
		os << resource << ":";
		for(const vector<int> & oc : occupancies){
			if(comma)
				os << ",";
			else
				comma=true;

			os << "(";
			bool comma2=false;
			for(int i : oc){
				if(comma2)
				os << ",";
			else
				comma2=true;
				os << i;
			}

			os << ")";

		}
		os << "\"];" << endl;


		for(NBDD * nb : truechildren)
			os << "n_" << id << " -> n_" << nb->id << " [label=\"1\"];" << endl;

		os << "n_" << id << " -> n_" << falsechild->id << " [label=\"0\"];" << endl;
	}
}

