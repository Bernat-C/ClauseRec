#ifndef BACP_DEFINITION
#define BACP_DEFINITION
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <set>
using namespace std;

class BACP
{

    int ncourses; //number of courses
    int nperiods; //number of periods
    vector<int> load;
    int minLoad;
    int maxLoad;
    int minCourses;
    int maxCourses;
    vector<pair<int,int> > preq;
    
public:
    BACP(int ncourses, int nperiods);
    int getNCourses() const;
    int getNPeriods() const;
    int getMinLoad() const;
    int getMaxLoad() const;
    int getMinCourses() const;
    int getMaxCourses() const;
    
    void setLoad(int course, int load);
    int getLoad(int course) const;
    
    void addPrerequirement(int pred, int suc);
    const vector<int> & getPrerequirements() const;
    
    
    friend ostream &operator <<(ostream &, BACP &);
    
    ~BACP();
};

#endif


