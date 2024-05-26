#include "limits.h"
#include "bacp.h"

using namespace std;

BACP::BACP(int ncourses, int nperiods, int minLoad, int maxLoad, int minCourses, int maxCourses){
    this->ncourses = ncourses;
    this->nperiods = nperiods;
    this->minLoad = minLoad;
    this->maxLoad = maxLoad;
    this->minCourses = minCourses;
    this->maxCourses = maxCourses;
    load.resize(ncourses);
}

BACP::~BACP(){

}

int BACP::getNCourses() const{
    return ncourses;
}

int BACP::getNPeriods() const{
    return nperiods;
}

int BACP::getMinLoad() const{
    return minLoad;
}

int BACP::getMaxLoad() const{
    return maxLoad;
}

int BACP::getMinCourses() const{
    return minCourses;
}

int BACP::getMaxCourses() const{
    return maxCourses;
}

void BACP::setLoad(int course, int load){
    this->load[course] = load;
}

int BACP::getLoad(int course) const{
    return load[course];
}

void BACP::addPrerequirement(int pred, int suc){
    preq.push_back(pair<int,int>(pred,suc));
}

const BACP::vector<int> & getPrerequirements() const{
    return preq;
}

void BACP::computeCover(vector<vector<int> > & cover) const {
    
}

ostream &operator <<(ostream & os, BACP & a){

}

