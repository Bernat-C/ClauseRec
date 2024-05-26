#include "limits.h"
#include "mspsp.h"
#include "util.h"
#include "bipgraph.h"
#include "disjointset.h"
#include <algorithm>
#include <cmath>
using namespace std;

MSPSP::MSPSP(int nactivities, int nresources, int nskills){
	this->nactivities = nactivities;
	this->nresources = nresources;
	this->nskills = nskills;

	resHasSkill = new bool * [nresources];
	for(int k = 0; k < nresources; k++){
		resHasSkill[k] = new bool[nskills];
		for(int l = 0; l < nskills; l++)
			resHasSkill[k][l] = false;
	}

	actSkillDemand.resize(nactivities+2);
	succs = new vector<int> [nactivities+2];
	extPrecs = new int * [nactivities+2];
	duration = new int [nactivities+2];

	nResourcesMastering = new int [nskills];

	dominatedSets = vector<bool>(1<<nskills,false);

	for(int i = 0; i < nactivities+2; i++){
		duration[i] = 0;
		actSkillDemand[i] = vector<int>(nskills,0);
		extPrecs[i] = new int[nactivities+2];
		for(int j = 0; j < nactivities+2;j++) {
		    extPrecs[i][j] = INT_MIN;
        }
	}
}

MSPSP::~MSPSP(){
	delete [] succs;
	delete [] duration;
	for(int i = 0; i < nactivities+2; i++)
		delete [] extPrecs[i];
	for(int k = 0; k < nresources; k++)
		delete [] resHasSkill[k];

	delete [] nResourcesMastering;

	delete [] resHasSkill;
	delete [] extPrecs;
	for(NBDD * n : nbdds)
		delete n;
}

int MSPSP::getNActivities() const{
	return nactivities;
}

int MSPSP::getNResources() const{
	return nresources;
}

int MSPSP::getNSkills() const{
	return nskills;
}

void MSPSP::computeNResourcesMastering(){
	for(int l = 0; l < nskills; l++){
		int res = 0;
		for(int k = 0; k < nresources; k++)
			if(resHasSkill[k][l])
				res++;
		nResourcesMastering[l] = res;
	}
}

int MSPSP::getNResourcesMastering(int l) const{
	return nResourcesMastering[l];
}

void MSPSP::getCapacities(vector<int> & capacities) const{
	capacities.resize(nskills);
	for(int l = 0; l < nskills; l++)
		capacities[l] = getNResourcesMastering(l);
}


void MSPSP::setDuration(int i, int p){
	duration[i] = p;
}

int MSPSP::getDuration(int i) const{
	return duration[i];
}

void MSPSP::setResourceSkill(int k, int l){
	resHasSkill[k][l] = true;
}

bool MSPSP::hasSkill(int k, int l) const{
	return resHasSkill[k][l];
}

int MSPSP::nMastered(int k) const{
	int sum = 0;
	for(int l = 0; l < nskills; l++)
		if(resHasSkill[k][l])
			sum++;
	return sum;
}

void MSPSP::setDemand(int i, int l, int d){
	actSkillDemand[i][l] = d;
}

int MSPSP::getDemand(int i, int l) const{
	return actSkillDemand[i][l];
}

const vector<int> & MSPSP::getDemands(int i) const{
	return actSkillDemand[i];
}

bool MSPSP::demandsSkill(int i, int l) const{
	return actSkillDemand[i][l] > 0;
}

int MSPSP::getTotalDemand(int i) const{
	int dem = 0;
	for(int l = 0; l < nskills; l++)
		dem+=actSkillDemand[i][l];

	return dem;
}

bool MSPSP::usefulToActivity(int k, int i) const{
	for(int l = 0; l < nskills; l++)
		if(resHasSkill[k][l] && actSkillDemand[i][l]>0)
			return true;

	return false;
}

void MSPSP::addSuccessor(int i, int j){
	succs[i].push_back(j);
}

const vector<int> & MSPSP::getSuccessors(int i) const {
	return succs[i];
}

void MSPSP::computeExtPrecs(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			extPrecs[i][j] = duration[i];
	util::floydWarshall(extPrecs,nactivities+2);
}

void MSPSP::recomputeExtPrecs(){
	util::floydWarshall(extPrecs,nactivities+2);
}

void MSPSP::computeEnergyPrecedences(){
	list<int> q;
	bool * visited = new bool[nactivities+2];

	//Iterate all pairs of activities joined by a path
	for(int i = 0; i < nactivities+1; i++){
		for(int j = 0; j < nactivities+2; j++){
			if(i!=j && extPrecs[i][j]>INT_MIN){

				//Obtain activities that must be running between the execution of 'i' and 'j', according to precedences
				for(int k = 0; k < nactivities+2; k++)
					visited[k]=false;
				list<int> activities_between;

				q.insert(q.end(),succs[i].begin(),succs[i].end());
				while(!q.empty()){
					int k = q.front();
					q.pop_front();
					if(!visited[k] && extPrecs[k][j] > INT_MIN){
						visited[k] = true;
						activities_between.push_back(k);
						q.insert(q.end(),succs[k].begin(),succs[k].end());
					}
				}

				int rl = extPrecs[i][j];

				int nCombinations = 1 << nskills;

				//Iterate all non-dominated sets of skills
				for(int combination = 1; combination < nCombinations; combination++){ //Has to be read as a binary number

					//If the combination is not dominated (skip unnecessary checks)
					if(!dominatedSets[combination])
					{
						int dem = 0;
						int nres = 0;
						for(int l = 0; l < nskills; l++){
							if(util::nthBit(combination,l)){
								for(int k : activities_between)
									dem += actSkillDemand[k][l] * duration[k];
								nres += nResourcesMastering[l];
							}
						}
						
						double energy = dem / (double) nres;
						int lag = ((int) ceil(energy)) + duration[i];
						if(lag > rl)
							rl = lag;
					}
				}

				//Update lag if needed
				if(rl>extPrecs[i][j]){
					energy_precs.push_back(pair<pair<int,int>,int>(pair<int,int>(i,j),rl));
					extPrecs[i][j]=rl;
				}
			}
		}

	}
	//cout << "c NOMBRE EXTESES: " << nenergyprecs << endl;
	delete [] visited;
}

int MSPSP::getExtPrec(int i, int j) const{
	return extPrecs[i][j];
}

bool MSPSP::isPred(int i, int j) const{
	return extPrecs[i][j]!= INT_MIN;
}

bool MSPSP::inPath(int i, int j) const{
	return isPred(i,j) || isPred(j,i);
}

int MSPSP::trivialUB() const{
	int UB = 0;
	for(int i = 0; i < nactivities+2; i++)
		UB+=duration[i];
	return UB;
}

int MSPSP::trivialLB() const{
	return ES(nactivities+1);
}

int MSPSP::ES(int i) const{
	return extPrecs[0][i];
}

int MSPSP::LS(int i, int UB) const{
	return UB - extPrecs[i][nactivities+1];
}

int MSPSP::EC(int i) const{
	return extPrecs[0][i]+duration[i];
}

int MSPSP::LC(int i, int UB) const{
	return UB - extPrecs[i][nactivities+1] + duration[i];
}


void MSPSP::makeDurationsUnitary(){
	for(int i = 1; i <= nactivities; i++)
		duration[i]=1;
}


//The most specialized skill is mastered by at least 2 resources
//Kind of each workers spread evenly, giving highest priority to the less specialized
//The highest demand on a skill is smaller or equal than the number of resources able to perform it
void MSPSP::makeResourceMasteryGerarquic(){

	vector<int> initdist(nskills);
	vector<int> newdist(nskills);

	//Compute the initial distributon of skill mastery
	for(int s = 0; s < nskills; s++)
		initdist[s] = getNResourcesMastering(s);
	

	//Compute the new distributon of skill mastery
	if(nresources <= nskills){
		for(int i = 0; i < nskills-nresources; i++)
			newdist[i]=0;
		for(int i = nskills-nresources; i < nresources; i++)
			newdist[i]=0;
	}
	else{
		int avg = (nresources-1)/nskills;
		for(int i = 0; i < nskills; i++){
			newdist[i]=avg;
		}
		int placed = avg*nskills;

		if(newdist[0]<2){
			newdist[0]++;
			placed++;
		}


		int toplace = nresources-placed;
		for(int i = nskills-toplace; i < nskills; i++){
			newdist[i]++;
		}

	}

	//Define the masterys according to the new distribution
	int res=0;
	for(int s=0; s < nskills; s++){
		for(int i = 0; i < newdist[s]; i++){
			for(int j = 0; j < s; j++)
				resHasSkill[res][j]=false;

			for(int j = s; j < nskills; j++)
				resHasSkill[res][j]=true;

			res++;
		}
	}

	//Compute cumulative distribution
	vector<int> cumulnewdist(nskills);
	cumulnewdist[0]=newdist[0];
	for(int i = 1; i < nskills; i++)
		cumulnewdist[i]=cumulnewdist[i-1]+newdist[i];


	//Get the capacity change ratio of each skill
	vector<float> change(nskills,0);
	for(int s = 0; s < nskills; s++){
		if(initdist[s]!=0){
			change[s] = cumulnewdist[s] / (float)initdist[s];
		}
	}

	//Readjust the demands proportionally to the number of resources
	// and truncate the demands to the cumulative distribution to avoid unsat
	for(int s = 0; s < nskills; s++){
		if(change[s]!=0){
			for(int i = 1; i <= nactivities; i++){
				actSkillDemand[i][s] = (int) ceil(actSkillDemand[i][s]*change[s]);
				if(actSkillDemand[i][s]>cumulnewdist[s])
					actSkillDemand[i][s]=cumulnewdist[s];
			}
		}
	}
	
}

int MSPSP::getNResourceTypes() const{
	return res_types.size();

}


const vector<int> & MSPSP::getResourcesOfType(int t) const{
	return res_types[t];
}


const vector<int> & MSPSP::getNBDDOrderedResources(int i) const{
	return nbddOrderedResources[i];
}


void MSPSP::preProcess(){
	computeNResourcesMastering();
	computeDominatedSets();
	computeExtPrecs();
	computeResTypes();
	computeNBDDOrder();
}

void MSPSP::preProcessNBDD(){
	computeNBDDs();
}

void MSPSP::computeResTypes(){
	map<vector<bool>, vector<int> > m;

	for (int k=0;k<nresources;k++)
	{
		vector<bool> v(nskills);
		for(int l = 0; l < nskills; l++)
			v[l]=resHasSkill[k][l];
		m[v].push_back(k);
	}
	for(const pair<vector<bool>,vector<int> > & p: m)
		res_types.push_back(p.second);

	std::sort(res_types.begin(),res_types.end(),
			[&](const vector<int> & lhs, const vector<int> & rhs){
				return this->nMastered(lhs[0]) < this->nMastered(rhs[0]);
			}
		);
}


void MSPSP::computeNBDDOrder(){
	nbddOrderedResources.resize(nactivities+2);
	for(int i = 0; i <= nactivities+1; i++){
		for(int type = 0; type < getNResourceTypes(); type++){
			if(usefulToActivity(getResourcesOfType(type)[0],i))
			for(int k : getResourcesOfType(type))
				nbddOrderedResources[i].push_back(k);
		}
	}
}

void MSPSP::computeNBDDs(){
	nbdds.resize(nactivities+2);
	for(int i = 0; i <= nactivities+1; i++)
		nbdds[i] = NBDD::constructNBDD(this,nbddOrderedResources[i],actSkillDemand[i]);
}

NBDD * MSPSP::getNBDD(int i) const{
	return nbdds[i];
}


bool MSPSP::getAssignment(vector<int> & dem, int demsum, vector<bool> & fr, int k, int rescount, std::set<std::vector<int> > & U, vector<pair<int,int> > & used_res){
    

    while(k < nresources && fr[k]){
        k++;
    }
    
    
    //Check if the objective is met
    if(demsum == 0){
        return true;
    }
    
    //Check if trivially unsatisfiable
    if(demsum > rescount){
        return false;
    }
    //cerr << k << endl;
    
    fr[k]=true;
    
    vector<int> p(dem);
    for(int i = 0; i < nresources; i++)
        p.push_back(fr[i] ? 1 : 0);
    if(U.find(p) == U.end()){
        bool feas = getAssignment(dem,demsum,fr,k+1,rescount-1,U,used_res);
        if(feas){
            fr[k]=false;
            return true;
        }
    }
    
    fr[k]=true;
    
    for(int l = 0; l < nskills; l++){
        if(resHasSkill[k][l] & dem[l]>0){
            dem[l]--;
            p[l]--;
            if(U.find(p) == U.end()){
                bool feas = getAssignment(dem,demsum-1,fr,k+1,rescount-1,U,used_res);
                if(feas){
                    used_res.push_back(pair<int,int>(k,l));
                    return true;
                }
            }
            dem[l]++;
            p[l]++;
        }
    }
    fr[k]=false;
    p[nskills+k]=0;
    U.insert(p);
    return false;
}

int MSPSP::next_activity(const vector<vector<int> > & ordered_res, const vector<vector<int> > & predecessors,
	set<int> & remaining,const set<int> & C, vector<bool> & occupation,
	vector<pair<int,int> > & used_res, std::set<std::vector<int> > & U)
{
	set<int>::iterator it=remaining.begin();
	while (it!=remaining.end()) {
		bool correct=true;

		//Check if the activity can be scheduled according to precedences
		int i = *it;

		for(int j : predecessors[i]){
			if (C.find(j)==C.end()){
				correct=false;
				break;
			}
		}

		if (correct) {
			//At this point, we know that the activity can be scheduled by predecences
			//Now check if we can find a resource assignment to fulfill the demands
			vector<int> activityOccupation;
			for(int k : getNBDDOrderedResources(i))
				activityOccupation.push_back(occupation[k] ? 0 : -1);
			
            used_res.clear();

            vector<int> dem = actSkillDemand[i];
            int demsum = 0;
            for(int l = 0; l < nskills; l++)
                demsum+=dem[l];
            int rescount = 0;
            for(int k = 0; k < nresources; k++)
                if(!occupation[k])
                    rescount++;
            
            correct = getAssignment(dem, demsum, occupation, 0, rescount, U, used_res);
		}
		if (correct) {
			remaining.erase(it);
			return i;
		}
		else it++;
	}
	return -1;
}


//Parallel scheduling scheme: find a greedy solution
int MSPSP::computePSS(vector<int> & starts, vector<vector<pair<int,int> > > & assignment){
	
	//Start time and resource assignments of each activity
	assignment.resize(nactivities+2);
	starts.resize(nactivities+2);

    //Structure to store the bad resource assignments, dynamic programming
    std::set<std::vector<int> > U;

	vector<pair<int,int> > used_res;

	//Predecessor of each activity
	vector<vector<int> > predecessors(nactivities+2);
	for (int i=0;i<nactivities+2;i++)
		for (int j : succs[i])
			predecessors[j].push_back(i);

	//Number of skills that masters each resource
	vector<int> skillsByRes(nresources,0);
	for(int k = 0; k < nresources; k++)
		for(int l = 0; l < nskills; l++)
			if(resHasSkill[k][l])
				skillsByRes[k]++;

	//For each skill, the resources that masters it in ascending order of diversity
	vector<vector<int> > ordered_res(nskills);
	for(int l = 0; l < nskills; l++){
		for(int k = 0; k < nresources; k++) 
			if(resHasSkill[k][l])
				ordered_res[l].push_back(k);

		std::sort(ordered_res[l].begin(),ordered_res[l].end(),
			[&](int lhs, int rhs){
				return skillsByRes[lhs] < skillsByRes[rhs];
			}
		);
	}


	//A: set of activities running at the watched time
	//C: set of activities already completed at the watched time
	//remaining: activities not scheduled yet
	set<int> A,C,remaining;
	

	//Occupation of each resource
	vector<vector<bool> > occupation(1,vector<bool>(nresources,false));
	int t=0;
	int d=0;
	for (int i=1; i<nactivities+2; i++) 
		remaining.insert(i);

	starts[0]=0; //Add dummy start
	A.insert(0);

	//While not all the activities are scheduled
	while (A.size()+C.size()!=nactivities+2) {
		
		//If even with no activity scheduled we could not schedule a new activity, return fail
		if (A.empty())
				return -1;
		else {
			int antt=t;

			//Find the time where the first activity in A finishes
			t=INT_MAX;
			for (int ii : A)
				if(starts[ii]+duration[ii]<t)
					t=starts[ii]+duration[ii];

			//Move from A to C all the activities that finish at time t
			set<int> aesborrar;
			for (int ii : A){
				if (starts[ii]+duration[ii]==t){
					C.insert(ii);
					aesborrar.insert(ii);
				}
			}

			for(int jj: aesborrar)
				A.erase(jj);

			//Remove the already passed time instants from the occupation matrix
			//Position 0 will point to the watched time instant
			for (int i=antt;i<t;i++)
				occupation.erase(occupation.begin());

			//Get new activity to schedule
            if(occupation.empty())
                occupation.push_back(vector<bool>(nresources,false));
			d=next_activity(ordered_res,predecessors,remaining,C,occupation[0],used_res,U);
		}

		while (d!=-1) {
			starts[d]=t;
			assignment[d]=used_res;
			A.insert(d);
			for(int i=0;i<duration[d];i++) {
				if (occupation.size()<=i)
					occupation.push_back(vector<bool>(nresources,false));
				for(const pair<int,int> & p: used_res)
					occupation[i][p.first] = true;
			}

			//Get new activity to schedule
			d=next_activity(ordered_res,predecessors,remaining,C,occupation[0],used_res,U);
		}
	}

	return t;
}



void MSPSP::computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups) const{

  vector<pair<int,int> > matching;
  BipGraph bg(vtasks.size()+1,vtasks.size()+1);

  matching.clear();

  for(int i = 0; i < vtasks.size(); i++){
    for(int j = 0; j < vtasks.size(); j++){
      if(i!=j && getDuration(vtasks[i]) > 0){
			if(isPred(vtasks[i],vtasks[j]))
				bg.addEdge(i+1,j+1);
      }
    }
  }

  bg.hopcroftKarp(matching);

  DisjointSet s(vtasks.size());
  for(const pair<int,int> & edge : matching)
    s.join(edge.first-1,edge.second-1);

  s.getSets(groups,vtasks);
}

void MSPSP::computeDominatedSets(){
	int nSubsetsOfSkills = 1 << nskills;

	//All subsets organized by cardinality
	vector<list<int> >subsetsCard(nskills+1);
	subsetsCard[0].push_back(0);
	

	vector<int> capacity(nSubsetsOfSkills,0);
	capacity[0]=nresources;

	for(int c = 1; c < nSubsetsOfSkills; c++){
		subsetsCard[util::cardinality(c)].push_back(c);
		for(int k = 0; k < nresources; k++){
			for(int l = 0; l < nskills; l++)
			if(util::nthBit(c,l) && resHasSkill[k][l]){
				capacity[c]++;
				break;
			}
		}
	}


	//Iterate pairs of candidate dominances by difference 1 in cardinality (avoid all pairwise combinations)
	for(int card = nskills; card > 1; card--){
		for(int c1 : subsetsCard[card-1]){
			for(int c2 : subsetsCard[card]){
				if(capacity[c2]==capacity[c1] && c2 | c1 == c2){ //if c2 is superset of c1 and they have the same capacity
					dominatedSets[c1]=true;
					break;
				}
			}
		}
	}
	dominatedSets[0] = true;

}

bool MSPSP::isDominated(int subset){
	return dominatedSets[subset];
}




void MSPSP::printSolution(ostream & os, const vector<int> & starts, const vector<vector<pair<int,int> > > & assignment){
	for(int i = 0; i < starts.size(); i++)
		os << "S_" << i << ":" << starts[i] << "; ";

	int cost = 0;
	for (int i=0;i<assignment.size();i++){
		for(const pair<int,int> & p : assignment[i]){
			os << "A:" << i << ":" << p.first << ":" << p.second << "; ";
		}
	}
}



void MSPSP::printJSON(int UB)
{
	cout << "{" << endl;

	cout << "\"UB\" : " << UB << "," << endl;

	cout << "\"nActivities\" : " << nactivities << "," << endl;

	cout << "\"nResources\" : " << nresources << "," << endl;

	cout << "\"nSkills\" : " << nskills << "," << endl;

	cout << "\"duration\" : [";
	for(int i = 0; i < nactivities+2; i++){
		cout << duration[i];
		if(i < nactivities+1)
			cout << ",";
	}
	cout << "],"<< endl;

	cout << "\"demand\" : [" << endl;
	for(int i = 0; i < nactivities+2; i++){
		cout << "[";
		for(int s = 0; s < nskills; s++){
			cout << actSkillDemand[i][s];
			if(s < nskills-1)
				cout << ",";
		}
		cout << "]";
		if(i < nactivities+1)
			cout << "," << endl;
	}
	cout << "],"<< endl;

	cout << "\"successors\" : [" << endl;
	for(int i = 0; i < nactivities+2; i++){
		cout << "[";
		for(int j = 0; j < nactivities+2; j++){
			if(find(succs[i].begin(),succs[i].end(),j)!=succs[i].end())
				cout << "1";
			else
				cout << "0";
			if(j < nactivities+1)
				cout << ",";
		}
		cout << "]";
		if(i < nactivities+1)
			cout << "," << endl;
	}
	cout << "],"<< endl;

	cout << "\"mastersSkill\" : [" << endl;
	for(int r = 0; r < nresources; r++){
		cout << "[";
		for(int s = 0; s < nskills; s++){
			if(resHasSkill[r][s])
				cout << "1";
			else
				cout << "0";
			if(s < nskills-1)
				cout << ",";
		}
		cout << "]";
		if(r < nresources-1)
			cout << "," << endl;
	}
	cout << "]"<< endl;

	cout << "}" << endl;
}


ostream &operator << (ostream &output, MSPSP &m)
{
	output << m.nactivities+2 << " " << m.nskills << " " << m.nresources << endl;
	for(int r = 0; r < m.nresources; r++){
		output << r;
		for(int l = 0; l < m.nskills; l++)
			if(m.hasSkill(r,l))
				output << " " << l;
		output << endl;
	}

	for(int i = 0; i < m.nactivities+2; i++){
		output << i << " " << m.getDuration(i);
		for(int l = 0; l < m.nskills; l++)
			output << " "  << m.getDemand(i,l);
		output << endl;
	}

	for(int i = 0; i < m.nactivities+2; i++){
		output << i;
		for(int i2 : m.getSuccessors(i))
			output << " " << i2;
		output << endl;
	}
	return output;
}

void MSPSP::printCTLPInstance(ostream & f, const int UB, bool useint) {
	f << "Minimize" << endl;
	f << "obj: " << endl;
	f << "q_" << getNActivities()+1 << endl;

	f << "Subject to" << endl;

	for(int i = 0; i < getNActivities()+2; i++)
		for(int i2 : getSuccessors(i)){
			f << "+ q_" << i2 << " - q_" << i << " >= "  << getDuration(i)  << endl;
		}

	for (int i=1; i<getNActivities()+1; i++)
		for (int j=i+1;j<getNActivities()+2; j++) 
			if (getExtPrec(i,j)<0 and getExtPrec(j,i)<0) {
				f << "+ q_" << j << " - q_" << i << " - " << UB+1 << " u_" << i << "_" << j << " >= "  << getDuration(i)-(UB+1)  << endl;
				f << "+ q_" << i << " - q_" << j << " - " << UB+1 << " u_" << j << "_" << i << " >= "  << getDuration(j)-(UB+1)  << endl;
			}

	for (int i=1; i<getNActivities()+1; i++)
		for (int j=i+1;j<getNActivities()+2; j++) 
			if (getExtPrec(i,j)<0 and getExtPrec(j,i)<0)
				f << "+ u_" << i << "_" << j << " + u_" << j << "_" << i  << " <= 1" << endl;

	for (int j=1; j<getNActivities()+1; j++)
		for (int l=0;l<getNSkills(); l++)
			if (getDemand(j,l)>0) {
				for (int k=0;k<getNResources(); k++)
					if (demandsSkill(j,l)>0 and hasSkill(k,l)) 
						f << "+ w_" << j << "_" << l << "_" << k << " ";
				f << "= " << getDemand(j,l) << endl;
			}


	for (int j=1; j<getNActivities()+1; j++)
		for (int k=0;k<getNResources(); k++) {
			bool entro=false;
			for (int l=0;l<getNSkills(); l++)
				if (demandsSkill(j,l)>0 and hasSkill(k,l)) {
					f << "+ w_" << j << "_" << l << "_" << k << " ";
					entro=true;
				}
			if (entro) 
				f << "<= 1" << endl;
		}

	for (int i=1; i<getNActivities()+1; i++)
		for (int j=i+1;j<getNActivities()+2; j++) if (getExtPrec(i,j)<0 and getExtPrec(j,i)<0)
			for (int k=0;k<getNResources(); k++) {
				bool entro=false;
				for (int l=0;l<getNSkills(); l++)
					if (demandsSkill(i,l)>0 and hasSkill(k,l)) {
						f << "+ w_" << i << "_" << l << "_" << k << " ";
						entro=true;
					}
				for (int l=0;l<getNSkills(); l++)
					if (demandsSkill(j,l)>0 and hasSkill(k,l)) {
						f << "+ w_" << j << "_" << l << "_" << k << " ";
						entro=true;
					}
				if (entro) 
					f << "- u_" << i << "_" << j << " - u_" << j << "_" << i  << " <= 1" << endl;
			}






	f << endl;
	f << "Bounds" << endl;
	f << 0 << " <= " << "q_" << 0 <<  " <= " << 0 << endl;
	for (int i=1; i<getNActivities()+1; i++) 
		f << ES(i) << " <= " << "q_" << i <<  " <= " << LS(i,UB) << endl;
	f << ES(getNActivities()+1) << " <= " << "q_" << getNActivities()+1 <<  " <= " << UB << endl;
	f << endl;

	if(useint){
		f << "General" << endl;
		for (int i=0; i<getNActivities()+2; i++) 
			f << "q_" << i << " ";
		f << endl;
		f << endl;
	}

	f << "Binary" << endl;
	for (int i=1; i<getNActivities()+1; i++) {
		for (int j=i+1;j<getNActivities()+2; j++) if (getExtPrec(i,j)<0 and getExtPrec(j,i)<0) {
			f << "u_" << i << "_" << j << " ";
			f << "u_" << j << "_" << i << " ";
		}
		f << endl;
	}

	for (int j=1; j<getNActivities()+1; j++) {
		for (int l=0;l<getNSkills(); l++)
			if (getDemand(j,l)>0)
				for (int k=0;k<getNResources(); k++)
					if (demandsSkill(j,l)>0 and hasSkill(k,l)) 
						f << "w_" << j << "_" << l << "_" << k << " ";
		f << endl;
	}
} 

void MSPSP::printDTLPInstance(ostream & f, const int UB, bool dt) {
	f << "Minimize" << endl;
	f << "obj: " << endl;
    for (int t=ES(getNActivities()+1); t<=UB; t++)
        f << "+ " << t << " S_" << getNActivities()+1 <<  "_" << t << " ";
    f << endl;

	f << "Subject to" << endl;

	f << "S_0_0 = 1" << endl;
	for (int j=1; j<getNActivities()+1; j++) {
        for (int t=ES(j); t<=LS(j,UB); t++)
            f << "+ S_" << j <<  "_" << t << " ";
        f << " = 1" << endl;
    }
    for (int t=ES(getNActivities()+1); t<=UB; t++)
        f << "+ S_" << getNActivities()+1 <<  "_" << t << " ";
    f << " = 1" << endl;

    if (dt) { //DT
        for(int i = 0; i < getNActivities()+2; i++) {
            int esi=ES(i);
            int lsi=LS(i,UB);
            if (i==0) {esi=0; lsi=0;} else if (i==getNActivities()+1) lsi=UB;

			for(int j : getSuccessors(i)){
                int esj=ES(j);
                int lsj=LS(j,UB);
                if (j==0) {esj=0; lsj=0;} else if (j==getNActivities()+1) lsj=UB;
                for (int t=esj; t<=lsj; t++)
                    f << "+ " << t << " S_" << j << "_" << t << " ";
                for (int t=esi; t<=lsi; t++)
                    f << "- " << t << " S_" << i << "_" << t << " ";
                f << ">= " << getDuration(i) << endl;
            }
        }
    } else { //DDT
        for(int i = 0; i < getNActivities()+2; i++) {
            int esi=ES(i);
            int lsi=LS(i,UB);
            if (i==0) {esi=0; lsi=0;} else if (i==getNActivities()+1) lsi=UB;
            for(int j : getSuccessors(i)){
                int esj=ES(j);
                int lsj=LS(j,UB);
                if (j==0) {esj=0; lsj=0;} else if (j==getNActivities()+1) lsj=UB;
                for (int t=esi; t<=lsi; t++) {
                    for (int t1=t; t1<=lsi; t1++)
                        f << "+ S_" << i << "_" << t1 << " ";
                    int lsf=lsj;
                    if (t+getDuration(i)-1 < lsf) lsf=t+getDuration(i)-1;
                    for (int t1=esj; t1<=lsf; t1++)
                        f << "+ S_" << j << "_" << t1 << " ";
                    f << "<= 1" << endl;
                }
            }
        }
    }



    for (int j=1; j<getNActivities()+1; j++)
        for (int l=0;l<getNSkills(); l++)
            if (getDemand(j,l)>0) {
                for (int t=ES(j); t<=LS(j,UB); t++)
                    for (int k=0;k<getNResources(); k++)
                        if (demandsSkill(j,l)>0 and hasSkill(k,l)) f << "+ z_" << j << "_" << l << "_" << k << "_" << t << " ";
                    f << "= " << getDemand(j,l) << endl;
            }


    for (int j=1; j<getNActivities()+1; j++)
        for (int k=0;k<getNResources(); k++)
            for (int t=ES(j); t<=LS(j,UB); t++) {
                bool entro=false;
                for (int l=0;l<getNSkills(); l++)
                    if (demandsSkill(j,l)>0 and hasSkill(k,l)) {
                        f << "+ z_" << j << "_" << l << "_" << k << "_" << t << " ";
                        entro=true;
                    }
                if (entro)
                    f << "- S_" << j << "_" << t << "<= 0" << endl;

            }

    for (int k=0;k<getNResources(); k++) //17
        for (int t=0; t<=UB; t++) {
            bool entro=false;
            for (int j=1; j<getNActivities()+1; j++) {
                int esf=ES(j);
                if (esf<t-getDuration(j)+1) esf=t-getDuration(j)+1;
                int lsf=LS(j,UB);
                if (lsf>t) lsf=t;
                for (int t1=esf; t1<=lsf; t1++)
                    for (int l=0;l<getNSkills(); l++)
                        if (demandsSkill(j,l)>0 and hasSkill(k,l)) {
                            f << "+ z_" << j << "_" << l << "_" << k << "_" << t1 << " ";
                            entro=true;
                        }
            }
            if (entro)
                f << "<= 1" << endl;

        }






	f << "Binary" << endl;

	f << "S_0_0" << endl;
    for (int j=1; j<getNActivities()+1; j++) {
        for (int t=ES(j); t<=LS(j,UB); t++)
            f << "S_" << j <<  "_" << t << " ";
        f << endl;
    }
    for (int t=ES(getNActivities()+1); t<=UB; t++)
        f << "S_" << getNActivities()+1 <<  "_" << t << " ";
    f << endl;



	for (int j=1; j<getNActivities()+1; j++) {
        for (int l=0;l<getNSkills(); l++)
            if (getDemand(j,l)>0)
                for (int k=0;k<getNResources(); k++)
                    if (demandsSkill(j,l)>0 and hasSkill(k,l)) {
                        for (int t=ES(j); t<=LS(j,UB); t++)
                            f << "z_" << j << "_" << l << "_" << k << "_" << t << " ";
                        f << endl;
                    }
        f << endl;
    }

 }


 void MSPSP::printJCLPInstance(ostream & f, const int ub, bool implicates, bool useint,bool reif) {

    int M=ub+1;
	f << "Minimize" << endl;
	f << "obj: " << endl;
	f << "S_" << getNActivities()+1 << endl;

	f << "Subject to" << endl;



	for (int i=0; i<getNActivities()+1; i++) //10 i 11
		for (int j=1;j<getNActivities()+2; j++)
			if (getExtPrec(i,j)>=0)
                    f << "+ S_" << j << " - S_" << i << " >= "  << getExtPrec(i,j)  << endl;

	for (int i=1; i<getNActivities()+1; i++) //12
		for (int k=0;k<getNResources(); k++){
            string ars="";
			for (int l=0;l<getNSkills(); l++)
				if (demandsSkill(i,l)>0 and hasSkill(k,l)){
					f << "+ ar_" << i << "_" << k << " - ars_" << i << "_" << k << "_" << l << " >= 0" << endl;
					ars += " + ars_" + to_string(i) + "_" + to_string(k) + "_" + to_string(l);
				}
            if (reif)
                f << "- ar_" << i << "_" << k << ars << " >= 0" << endl;
		}


	for (int i=1; i<getNActivities()+1; i++) //13
        for (int t=ES(i);t<=LC(i,ub); t++) {
            f << "+ " << M << " y_" << i << "_" << t <<
                " + S_" << i << " > " << t << endl;
            f << "+ " << M << " z_" << i << "_" << t <<
                " - S_" << i << " >=" << getDuration(i)-t << endl;
            f << "+ x_" << i << "_" << t << " - y_" << i << "_" << t <<
                " - z_" << i << "_" << t <<  " >= -1" << endl;
            if (reif) {
                f << "+ " << M << " y_" << i << "_" << t <<
                    " + S_" << " <= " << M+t << endl;
                f << "- " << M << " z_" << i << "_" << t <<
                    " + S_" << i << " > " << t-M-getDuration(i) << endl;
                f << "- 2 x_" << i << "_" << t << " + y" << t <<
                    " + z_" << i << "_" << t <<  " >= 0" << endl;
            }
        }


	for (int i=1; i<getNActivities()+1; i++) //14
		for (int k=0;k<getNResources(); k++)
			for (int t=ES(i);t<=LC(i,ub); t++){
                f << "+ art_" << i << "_" << k << "_" << t << " - ar_" << i << "_" << k << " - x_" << i << "_" << t << " >= -1" << endl;
                if (reif)
                    f << "- 2 art_" << i << "_" << k << "_" << t << " + ar_" << i << "_" << k << " + x_" << i << "_" << t << " >= 0" << endl;
			}


 	for (int i=1; i<getNActivities()+1; i++) //15
        for (int l=0;l<getNSkills(); l++)
            if (getDemand(i,l)>0) {
                for (int k=0;k<getNResources(); k++)
					if (demandsSkill(i,l)>0 and hasSkill(k,l))
                         f << "+ ars_" << i << "_" << k << "_" << l << " ";
                f << "= " << getDemand(i,l) << endl;
            }


	for (int i=1; i<getNActivities()+1; i++) //16
		for (int k=0;k<getNResources(); k++) {
			bool entro=false;
			for (int l=0;l<getNSkills(); l++)
				if (demandsSkill(i,l)>0 and hasSkill(k,l)) {
					f << "+ ars_" << i << "_" << k << "_" << l << " ";
					entro=true;
				}
			if (entro)
				f << "<= 1" << endl;
		}



	for (int k=0;k<getNResources(); k++) //17
		for (int t=0;t<=ub; t++) {
            bool dins=false;
            for (int i=1; i<getNActivities()+1; i++)
                if (t>=ES(i) and t<=LC(i,ub)) {
                    bool entro=false;
                    for (int l=0;l<getNSkills(); l++) {
                        if (demandsSkill(i,l)>0 and hasSkill(k,l) and !entro) {
                            entro=true;
                            dins=true;
                            f << "+ art_" << i << "_" << k << "_" << t << " ";
                        }
                    }
                }
            if (dins)
                f << "<= 1" << endl;
        }

    if(implicates) {
        //Implicate 2
        for(int l = 0; l < getNSkills(); l++){
            for(int t = 0; t < ub; t++){
                vector<vector<int> > X;
                vector<vector<int> > Q;

                vector<int> vtasks;
                vector<set<int> > groups;

                for (int i=1;i<=getNActivities();i++) {
                    int ESi=ES(i);
                    int LCi=LC(i,ub);
                    if (t>=ESi && t<LCi && demandsSkill(i,l))
                        vtasks.push_back(i);
                }

                if(!vtasks.empty())
                    computeMinPathCover(vtasks,groups);


                for (const set<int> & group : groups) {
                    vector<int> vars_part;
                    vector<int> coefs_part;

                    for(int i : group){
                        vars_part.push_back(i);
                        coefs_part.push_back(getDemand(i,l));
                    }

                    if(!coefs_part.empty()){
                        X.push_back(vars_part);
                        Q.push_back(coefs_part);
                    }
                }


                if (!X.empty()){
                    for (int ii=0; ii<Q.size(); ii++){
                        for (int iii=0; iii<Q[ii].size(); iii++)
                              f << " + " << Q[ii][iii] << " x_" << X[ii][iii] << "_" << t << " ";
                        f << "<= " << getNResourcesMastering(l);
                    }
                    //util::sortCoefsDecreasing(Q,X);
                    //f->addAMOPB(Q,X,ins->getNResourcesMastering(l),sargs->getAMOPBEncoding());
                }
            }
        }





    }





	f << endl;
	f << "Bounds" << endl;
	f << 0 << " <= " << "S_" << 0 <<  " <= " << 0 << endl;
	for (int i=1; i<getNActivities()+1; i++)
		f << ES(i) << " <= " << "S_" << i <<  " <= " << LC(i,ub) << endl;
	f << ES(getNActivities()+1) << " <= " << "S_" << getNActivities()+1 <<  " <= " << ub << endl;
	f << endl;


    f << "General" << endl;
    if(useint){
    	for (int i=0; i<getNActivities()+2; i++)
        	f << "S_" << i << " ";
    	f << endl;
	}




	f << "Binary" << endl;
	for (int i=1; i<getNActivities()+1; i++) {
 		for (int t=ES(i);t<=LC(i,ub); t++) {
			f << "x_" << i << "_" << t << " ";
            f << "y_" << i << "_" << t << " ";
            f << "z_" << i << "_" << t << " ";
        }
        f << endl;
	}


	for (int i=1; i<getNActivities()+1; i++) {
        for (int k=0;k<getNResources(); k++) {
            bool def=false;
            for (int l=0;l<getNSkills(); l++)
					if (demandsSkill(i,l)>0 and hasSkill(k,l)) {
                        if (!def) {
                            f << "ar_" << i << "_" << k << " ";
                            for (int t=ES(i);t<=LC(i,ub); t++)
                                f << "art_" << i << "_" << k << "_" << t << " ";
                            def=true;
                        }
                        f << "ars_" << i << "_" << k << "_" << l << " ";
					}
		f << endl;
        }
    }
}

void MSPSP::printDZN(ostream & os, int maxt_heur) const{
	os << endl;

	os << "mint_energy = " << trivialLB() << ";" << endl << endl;

	os << "maxt_heur = " << maxt_heur << ";" << endl << endl;

	os << "n_energyprec = " << energy_precs.size() << ";" << endl << endl;

	
	if(energy_precs.empty())
		os << "energy_prec = array2d(1..0,1..3,[]);" << endl << endl;
	else{
		os << "energy_prec = [";
		for(const pair<pair<int,int>,int> &p : energy_precs){
			os << "| " << (p.first.first+1) << "," << (p.first.second+1) << ","  << (p.second) << "," << endl;
		}
		os << "|];" << endl << endl;
	}

	int nUndominatedSets = 0;
	int nSubsetsOfSkills = 1<<nskills;

	os << "subsets_implied = [ " << endl;
	bool comma = false;
	for(int i = 1; i < nSubsetsOfSkills; i++){
		if(!dominatedSets[i]){
			nUndominatedSets++;

			if(comma)
				os << "," << endl;
			else
				comma=true;
			bool comma2=false;
			os << "{";
			for(int l = 0; l < nskills; l++)
			{
				if(util::nthBit(i,l)){
					if(comma2)
						os << ",";
					else
						comma2=true;
					os << (l+1);
				}
			}
			os << "}";
		}
	}
	os << "];" << endl << endl;

	os << "n_implied = " << nUndominatedSets << ";" << endl << endl;
}
