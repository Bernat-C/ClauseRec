#include "clinics.h"
#include "limits.h"
#include <list>
#include <algorithm>
#include "util.h"
#include <math.h>
#include "errors.h"
#include <stdlib.h>

using namespace std;

Clinics::Clinics(int ntests, int ntypes, int nactivities, int nresources){

	//Prepare instance data

	this->nactivities = nactivities;
	this->nresources = nresources;
	this->ntests = ntests;
	this->ntypes = ntypes;

	this->test_sample = new int[ntests];
	this->test_type = new string[ntests];

	this->activity_test = new int[nactivities+2];
	this->activity_id = new int[nactivities+2];
	this->activity_type = new int[nactivities+2];
	this->clean_factor = new string[nactivities+2];


	this->type_activity = new vector<int>[ntypes];
	this->demands = new vector<pair<int,int> >[nresources];
	this->ons = new vector<pair<int,int> >[nactivities+2];
	this->offs = new vector<pair<int,int> >[nactivities+2];

	this->succs = new vector<int>[nactivities+2];
	this->preds = new vector<int>[nactivities+2];
	this->ext_precs = new int*[nactivities+2];
	this->need_clean = new bool*[nactivities+2];

	for(int i = 0; i < nactivities+2; i++){
		this->ext_precs[i] = new int[nactivities+2];
		this->need_clean[i] = new bool[nactivities+2];
	}

	this->duration = new int[nactivities+2];
	this->clean_duration = new int[nactivities+2];
	this->capacity = new int[nresources];

	for(int i = 0; i < nactivities+2; i++){
		for(int j = 0; j < nactivities+2; j++){
			this->ext_precs[i][j] = INT_MIN;
			this->need_clean[i][j] = false;
		}
	}

	//Initialize dummy activities
	activity_test[0] = -1;
	activity_id[0] = -1;
	activity_type[0] = -1;
	clean_factor[0] = "";
	duration[0] = 0;
	clean_duration[0] = 0;

	activity_test[nactivities+1] = -1;
	activity_id[nactivities+1] = -1;
	activity_type[nactivities+1] = -1;
	clean_factor[nactivities+1] = "";
	duration[nactivities+1] = 0;
	clean_duration[nactivities+1] = 0;
}

Clinics::~Clinics(){
	delete [] test_sample;
	delete [] test_type;

	delete [] activity_test;
	delete [] activity_id;
	delete [] activity_type;
	delete [] clean_factor;
	delete [] succs;
	delete [] preds;
	delete [] duration;
	delete [] clean_duration;
	delete [] capacity;
	delete [] demands;
	delete [] ons;
	delete [] offs;
	delete [] type_activity;

	for(int i = 0; i < nactivities +2; i++){
		delete [] ext_precs[i];
		delete [] need_clean[i];
	}
	delete [] ext_precs;
	delete [] need_clean;
}

int Clinics::getNActivities() const{
	return nactivities;
}

int Clinics::getNResources() const{
	return nresources;
}

int Clinics::getActivitySample(int i) const{
	return test_sample[activity_test[i]];
}

int Clinics::getTestSample(int test) const{
	return test_sample[test];
}

void Clinics::setTestSample(int test, int sample){
	test_sample[test]=sample;
}

int Clinics::getId(int i) const{
	return activity_id[i];
}

void Clinics::setId(int i, int id){
	activity_id[i]=id;
}

string Clinics::getActivityTestType(int i) const{
	return test_type[activity_test[i]];
}

string Clinics::getTestType(int id) const{
	return test_type[id];
}

void Clinics::setTestType(int id, string s){
	test_type[id]=s;
}

int Clinics::getTest(int i) const{
	return activity_test[i];
}

void Clinics::setTest(int i, int test){
	activity_test[i]=test;
}

int Clinics::getType(int i) const{
	return activity_type[i];
}

void Clinics::setType(int i, int type){
	activity_type[i] = type;
	type_activity[type].push_back(i);
}

void Clinics::setDuration(int i, int p){
	duration[i] = p;
}

int Clinics::getDuration(int i) const{
	return duration[i];
}

void Clinics::setCleanDuration(int i, int p){
	clean_duration[i] = p;
}

int Clinics::getCleanDuration(int i) const{
	return clean_duration[i];
}

void Clinics::addSuccessor(int i, int j, int lag){
	succs[i].push_back(j);
	preds[j].push_back(i);
	ext_precs[i][j]=lag;
}

const vector<int> & Clinics::getSuccessors(int i) const {
	return succs[i];
}

const vector<int> & Clinics::getPredecessors(int i) const {
	return preds[i];
}


void Clinics::setCapacity(int r, int c){
	capacity[r]=c;
}

int Clinics::getCapacity(int r) const{
	return capacity[r];
}

void Clinics::setCleanFactor(int i, const string & s){
	clean_factor[i]=s;
}

string Clinics::getCleanFactor(int i) const{
	return clean_factor[i];
}

void Clinics::setNeedClean(int i , int j, bool b){
	need_clean[i][j]=b;
	need_clean[j][i]=b;
}

bool Clinics::needClean(int i, int j) const{
	return need_clean[i][j];
}

bool Clinics::needMutex(int i, int j) const{
	return activity_type[i]==activity_type[j] && !areOrdered(i,j);
}

void Clinics::addDemand(int i, int j, int r){
	demands[r].push_back(pair<int,int>(i,j));
	ons[i].push_back(pair<int,int>(j,r));
	offs[j].push_back(pair<int,int>(i,r));
}

const vector<pair<int,int> > & Clinics::getDemands(int r) const{
	return demands[r];
}

const vector<pair<int,int> > & Clinics::getOns(int i) const{
	return ons[i];
}

const vector<pair<int,int> > & Clinics::getOffs(int i) const{
	return offs[i];
}

int Clinics::getExtPrec(int i, int j) const{
	return ext_precs[i][j];
}

bool Clinics::areOrdered(int i, int j) const{
	return ext_precs[i][j] >= clean_duration[i] || ext_precs[j][i]>=clean_duration[j];
}


int Clinics::trivialUB() const{
	int ub = 0;
	for(int i = 0; i < nactivities+2; i++){
		int max_lag = clean_duration[i];
		for(int j : succs[i])
			if(ext_precs[i][j]>max_lag)
				max_lag = ext_precs[i][j];
		ub+=max_lag;
	}

	return ub;
}

int Clinics::trivialLB() const{
	return ES(nactivities+1);
}

int Clinics::ES(int i) const{
	return ext_precs[0][i] > 0 ? ext_precs[0][i] : 0;
}

int Clinics::LS(int i, int UB) const{
	return ext_precs[i][nactivities+1] > 0 ? UB - ext_precs[i][nactivities+1] : UB;
}

int Clinics::EC(int i) const{
	return ES(i) + clean_duration[i];
}

int Clinics::LC(int i, int UB) const{
	return LS(i,UB) + clean_duration[i];
}


void Clinics::computeExtPrecs(){
	util::floydWarshall(ext_precs,nactivities+2);
}



bool Clinics::needClean(const vector<pair<int,int> > & ordered, int i, int id, int type) const{
	for(int j = i+1; j < ordered.size(); j++){
		if(getType(ordered[j].first)==type)
			return needClean(id,ordered[j].first);
	}
	return false;
}

bool Clinics::needCleanAtTime(int * usage, int makespan, int t, int id) const{
	for(int j = t+1; j < makespan; j++){
		if(usage[j]!=0 && usage[j]!=id){
			if(usage[j]==-id)
				return false;
			else if(usage[j]>0)
				return needClean(id,usage[j]);
		}
	}
	return false;
}

void Clinics::buildMachineState(const vector<int> & starts, const vector<pair<int,int> > & ordered, int ** usage, int ** capacities, int makespan) const{

	//Compute machine states
	for(int i = 0; i < 3; i++)
		for(int t = 0; t < makespan; t++)
			usage[i][t]=0;

	for(int i = 0; i < 4; i++)
		for(int t = 0; t < makespan; t++)
			capacities[i][t]=getCapacity(i);


	for(int i = 1; i <= getNActivities(); i++){
		int id = ordered[i].first;
		int start = ordered[i].second;
		int type = getType(id);

		bool needclean = needClean(ordered,i,id,type);
		int dur = needclean ? getCleanDuration(id) : getDuration(id);

		for(int t = 0; t < dur; t++){
			if(t > getDuration(id) && !needclean && usage[type][start+t]==0)
				usage[type][start+t] = -id;
			else
				usage[type][start+t] = id;
		}

		for(const pair<int,int> & p : getOns(id)){
			int off = p.first;
			int r = p.second;
            for(int t = start; t < starts[off]+getDuration(off);t++)
				capacities[r][t]--;
		}
	}
}

bool Clinics::i_enforceActivity(vector<int> & starts, vector<pair<int,int> > & ordered,int ** usage, int ** capacities) const{
	int makespan = starts[getNActivities()+1];
	bool change = false;

	for(int i = 1; i <= getNActivities(); i++){
		int id = ordered[i].first;
		int start = ordered[i].second;
		int type = getType(id);

		for(int t = 0; t < start; t++){
			bool fits = true;

			//Check precedences
			for(int j : getPredecessors(id)){
				if(t-starts[j] < getExtPrec(j,id)){
					fits=false;
					break;
				}
			}


			//Check activity overlap
			if(fits){
				bool needclean = needCleanAtTime(usage[type],makespan,t,id);
				int dur = needclean ? getCleanDuration(id) : getDuration(id);

				for(int t2 = 0; t2 < dur; t2++){
					int running = usage[type][t+t2];
					if(running>0 && running!=id){
						fits=false;
						break;
					} else if(running<0){
						if(needClean(id,-running)){
							fits=false;
							break;
						}
					}
				}
			}

			//Check resources
			if(fits && type==2){
				for(const pair<int,int> & p : getOns(id)){
					int r = p.second;
                    for(int t2 = t; t2 < start; t2++){
						if(capacities[r][t2]==0){
							fits=false;
							break;
						}
                    }
                    if(!fits)
						break;
				}
			}


			if(fits){
				int previousStart = starts[id];
				starts[id]=t;
				change=true;


				//Update sorted starts
				for(pair<int,int> & p : ordered){
					if(p.first==id){
						p.second=starts[id];
						break;
					}
				}
				sort(ordered.begin()+1, ordered.end()-1,
					[ ]( const pair<int,int>& lhs, const pair<int,int>& rhs )
					{
						return lhs.second < rhs.second;
					}
				);

				//Update machine state
				buildMachineState(starts, ordered, usage, capacities,  makespan);
			}

		}

	}

	return change;
}


void Clinics::computeGreedySchedule(vector<int> & starts) const{
	starts.resize(nactivities+2);

	//Compute trivial schedule
	int start = 0;
	//For each test
    for(int test = 0; test < ntests; test++){
		//Schedule the activities of the test
		int last = INT_MIN;
		for(int j = 1; j <= getNActivities(); j++){
			if(activity_test[j]==test){
				starts[j]=start + getExtPrec(0,j);
				if(starts[j]+getDuration(j) > last)
					last=starts[j]+getDuration(j);
			}
		}
		start=last;
    }

    //Set dummy start times
    starts[0] = 0;
    starts[nactivities+1]=start;

	//Enforce activity
	enforceActivity(starts);
}


bool Clinics::enforceActivity(vector<int> & starts) const{
	int makespan = starts[getNActivities()+1];
	int min_start = *min_element(starts.begin()+1,starts.end()-1);
	for(int i = 1; i < getNActivities()+2; i++)
		starts[i]-=min_start;


	vector<pair<int,int> > ordered;

	for(int i = 0; i < getNActivities()+2; i++)
		ordered.push_back(pair<int,int>(i,starts[i]));

	sort(ordered.begin()+1, ordered.end()-1,
		[ ]( const pair<int,int>& lhs, const pair<int,int>& rhs )
		{
			return lhs.second < rhs.second;
		}
	);

	//Compute machine state
	int * usage[3];
	for(int i = 0; i < 3; i++)
		usage[i] = new int[makespan];

	int * capacities[4];
	for(int i = 0; i < 4; i++)
		capacities[i] = new int[makespan];

	buildMachineState(starts, ordered, usage, capacities,  makespan);

	//Apply activity until fix point
	int i = 0;
	bool changed = i_enforceActivity(starts,ordered,usage,capacities);
	bool somechanged=changed;

	while(changed)
		changed = i_enforceActivity(starts,ordered,usage,capacities);


	//Update the makespan
	makespan = INT_MIN;
	for(int i = 1; i <= getNActivities(); i++)
		if(starts[i]+getDuration(i) > makespan)
			makespan = starts[i]+getDuration(i);

	starts[getNActivities()+1]=makespan;

	for(int i = 0; i < 3; i++)
		delete [] usage[i];

	for(int i = 0; i < 4; i++)
		delete [] capacities[i];

	return somechanged;
}

void Clinics::printSolution(ostream & os, const vector<int> & starts) const {
	for(int i = 0; i < getNActivities()+2; i++)
		os << "S_" << i << "(" << (getTest(i)+1) << "_" << (getId(i)+1) << ")=" << starts[i] << "; ";
}


void Clinics::makeChart(const string & filename, const vector<int> & starts) const{
	int makespan = starts[getNActivities()+1];

	ofstream f;
	f.open((filename+"_").c_str());
	if (!f.is_open()) {
		cerr << "Could not create file " << filename+"_" << endl;
		exit(BADFILE_ERROR);
	}

	int height = 9 + (getCapacity(0) + getCapacity(1) + getCapacity(2) + getCapacity(3))/2;
	float resheight[getNResources()];

	f << "set term pdf size " << (makespan +10) << "," << height << " font \",60\"" << endl;
	f << "set output \"" << filename << "\"" << endl;
	f << "unset key" << endl;
	f << "set size ratio -1" << endl;
	f << "set xtics 0,1 rotate by 90 right" << endl;

	f << "set ytics (";
	f << "\"Sample Arm\" 0.5";
	f << ", \"Reagent Arm\" 1.5";
	f << ", \"Shuttle\" 2.5";
	float ypos = 4;
	resheight[0]=ypos;
	f << ", \"Incubator 1\" " << ypos;
	ypos+= 1 + getCapacity(0)/2.0;
	resheight[1]=ypos;
	f << ", \"Incubator 2\" " << ypos;
	ypos+= 1 + getCapacity(1)/2.0;
	resheight[2]=ypos;
	f << ", \"ORU\" " << ypos;
	ypos+= 1 + getCapacity(2)/2.0;
	resheight[3]=ypos;
	f << ", \"Predilution\" " << ypos;
	ypos+= 1 + getCapacity(3)/2.0;
	f << ")" << endl;

	f << "set xrange [0:" << makespan << "]" << endl;
	f << "set yrange [0:" << ((int)ypos+1) << "]" << endl;

	vector<pair<int,int> > ordered[3];

	for(int type = 0; type < 3; type++)
		for(int i = 1; i <= getNActivities(); i++)
			if(getType(i)==type)
				ordered[type].push_back(pair<int,int>(i,starts[i]));


	string colors[7] = {"#008080","#FF69B4","#EEEE00","#008000","#87CEFA","#FFA500","#FF4500"};

	for(int type = 0; type < 3; type++){
		sort(ordered[type].begin(), ordered[type].end(),
			[ ]( const pair<int,int>& lhs, const pair<int,int>& rhs )
			{
			   return lhs.second < rhs.second;
			}
		);

		for(int i = 0; i < ordered[type].size(); i++){
			const pair<int,int> & p = ordered[type][i];
			int id = p.first;
			int start = p.second;
			f << "set object " << id << " rect ";
			bool clean = i < ordered[type].size() - 1 && needClean(id,ordered[type][i+1].first);
			f << "from " << start << "," << (type+0.1);
			f << " to " << (start + (clean ? getCleanDuration(id) : getDuration(id))) << "," << (type+0.9);
			f << " fc rgb '" << colors[type] << "' front" << endl;

			f << "set label " << id << " \"" << getActivityTestType(id) << " " << getActivitySample(id) <<" " << (getId(id)+1);
			if(clean)
				f << " (C)";
			f << " (" << id << ")\" at " << (start+0.2) << "," << (type+0.5)<< " font \",50\" front" << endl;
		}
	}

	vector<tuple<int,int,int,int> > ordered_res;


	for(int r = 0; r < getNResources(); r++){
		for(const pair<int,int> & p : getDemands(r)){
			int start = starts[p.first];
			ordered_res.push_back(make_tuple(start,p.first,p.second,r));
		}
	}

	sort(ordered_res.begin(), ordered_res.end(),
			[ ]( const tuple<int,int,int,int> & lhs, const tuple<int,int,int,int> & rhs )
			{
			   return get<0>(lhs) < get<0>(rhs);
			}
		);

	int maxCap = INT_MIN;
	for(int i = 0; i < getNResources(); i++)
		if(getCapacity(i) > maxCap)
			maxCap = getCapacity(i);


	bool *** ocupacy = new bool **[getNResources()];
	for(int r = 0; r < getNResources(); r++){
		ocupacy[r] = new bool*[makespan];
		for(int t = 0; t < makespan; t++){
			ocupacy[r][t]=new bool[maxCap];
			for(int c = 0; c < maxCap; c++)
				ocupacy[r][t][c]=false;
		}
	}

	int id = getNActivities()+1;
	for(tuple<int,int,int,int> & p : ordered_res){
		int start = get<0>(p);
		int i = get<1>(p);
		int j = get<2>(p);
		int dur = starts[j]+getDuration(j) - starts[i];
		int r = get<3>(p);

		int slot = -1;
		bool full = true;
		while(full){
			slot++;
			full = ocupacy[r][start][slot];
		}
		for(int t = start; t < start+dur; t++)
			ocupacy[r][t][slot] = true;

		float h = resheight[r] + 0.5*slot;
		f << "set object " << id << " rect ";
		f << "from " << start << "," << (h);
		f << " to " << (start + dur) << "," << (h+0.5);
		f << " fc rgb '" << colors[3+r] << "' front" << endl;

		f << "set label " << id << " \"" << getActivityTestType(i) << " " << getActivitySample(i);
		f << " \" at " << (start+0.2) << "," << (h+0.2) << " font \",50\" front" << endl;
		id++;
	}

	f << "plot -1" << endl;


	for(int r = 0; r < getNResources(); r++){
		for(int t = 0; t < makespan; t++)
			delete [] ocupacy[r][t];
		delete [] ocupacy[r];
	}
	delete [] ocupacy;

	int res = system(("gnuplot < " + filename+"_").c_str());
    if (res==-1){
		cerr << "Could not run gnuplot!"<< endl;
		exit(UNSUPPORTEDFUNC_ERROR);
    }

    remove((filename+"_").c_str());

	f.close();

}

ostream &operator << (ostream &output, Clinics &m)
{
output << "N activities: " << m.nactivities << endl;
	output << "N resources: " << m.nresources << endl;
	output << "Capacities:";
	for(int r = 0; r < m.nresources; r++)
		output << " " << m.capacity[r];
	output << endl;
	for(int i = 0; i < m.nactivities+2; i++){
		output << "=== Activity " << i << " ===" << endl;
		output << "Activity test: " << m.activity_test[i] << endl;
		output << "Activity id: " << m.activity_id[i] << endl;
		output << "Duration: " << m.duration[i] << endl;
		output << "Clean duration: " << m.clean_duration[i] << endl;
		output << "Activity type: " << m.activity_type[i] << endl;
		output << "Clean factor: " << m.clean_factor[i] << endl;
		output << "Successors:" ;
		for(int j : m.succs[i])
			output << " " << j << " " << m.ext_precs[i][j];
		output << endl;
		output << "=================" << endl;
	}
	for(int r = 0; r < m.nresources; r++){
		output << "Demands " << r << ":";
		for(const pair<int,int> & p : m.demands[r])
			output << " (" << (p.first) << "," << (p.second) << ")";
		output << endl;
	}
	output << "====== Mutexes =======" << endl;
	for(int i = 1; i <= m.nactivities-1; i++){
		for(int j = i+1; j <= m.nactivities; j++){
			if(m.needMutex(i,j)){
				output << i << "(" << m.activity_test[i]+1 << "_" << m.activity_id[i]+1 << ")";
				output << "  <>  ";
				output << j << "(" << m.activity_test[j]+1 << "_" << m.activity_id[j]+1 << ")";
				if(m.needClean(i,j))
					output << "  (clean)";
				output << endl;
			}
		}
	}

	return output;
}
