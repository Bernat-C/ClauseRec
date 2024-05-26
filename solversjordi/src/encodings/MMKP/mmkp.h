#ifndef MMKP_H
#define MMKP_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>


using namespace std;

class MMKP
{
	
public:
	
	int n;
	int m;
	vector<int> l;
	
	vector<vector<int> > v;
	vector<vector<vector<int> > > r;
	vector<int> R;
	
	MMKP(int n, int m);
	
	~MMKP();
	
	
	int getLB();
	int getUB();

	void printESSENCEPrimeInstance();

	void shuffle();
	
	friend ostream &operator << (ostream &output, MMKP &m);
    
};

#endif

