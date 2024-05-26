#ifndef PARSER_DEFINITION
#define PARSER_DEFINITION

#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "prcpsp.h"
#include "mrcpsp.h"
#include "rcpspmax.h"
#include "mrcpspmax.h"
#include "rcpspt.h"
#include "mspsp.h"
#include "mmkp.h"
#include "clinics.h"
#include "clinicstest.h"
#include "auction.h"

using namespace std;

namespace parser
{

MRCPSP * parseMRCPSP(const string & filename);
MRCPSP * parseMRCPSPfromRCP(const string & filename);
MRCPSP * parseMRCPSPasPRCPSPfromRCP(const string & filename);
MRCPSP * parseMRCPSPfromDATA(const string & filename);
MRCPSP * parseMRCPSPfromPRB(const string & filename);
MRCPSP * parseMRCPSPfromMM(const string & filename);
MRCPSP * parseMRCPSPfromMM2(const string & filename);
void parseMRCPSPSolution(istream & str,MRCPSP * instance,vector<int> &starts, vector<int> &modes);

PRCPSP * parsePRCPSPfromRCP(const string & filename);
void parsePRCPSPSolution(istream & str,PRCPSP * instance,vector<int> &starts, vector<int> &modes, vector<int> &ends, vector<vector<int>> &is_exec);

RCPSPMAX * parseRCPSPMAX(const string & filename);
RCPSPMAX * parseRCPSPMAXfromSCH(const string & filename);
void parseRCPSPMAXSolution(istream & str,RCPSPMAX * instance,vector<int> &starts);

MRCPSPMAX * parseMRCPSPMAX(const string & filename);
MRCPSPMAX * parseMRCPSPMAXfromSCH(const string & filename);
void parseMRCPSPMAXSolution(istream & str,MRCPSPMAX * instance,vector<int> &starts, vector<int> &modes);

RCPSPT * parseRCPSPT(const string & filename);
RCPSPT * parseRCPSPTfromSMT(const string & filename);
void parseRCPSPTSolution(istream & str,RCPSPT * instance,vector<int> &starts);

MSPSP * parseMSPSP(const string & filename);
MSPSP * parseMSPSPfromTXT(const string & fileSname);
MSPSP * parseMSPSPfromDAT(const string & fileSname);
void parseMSPSPSolution(istream & str,MSPSP * instance,vector<int> &starts,  vector<vector<pair<int,int> > > & assignments);

MMKP * parseMMKP(const string & filename);
MMKP * parseMMKPfromMMKP(const string & filename);
MMKP * parseMMKPfromDATA(const string & filename);
MMKP * parseMMKPfromTXT(const string & filename);
void parseMMKPSolution(istream & str,MMKP * instance,vector<int> &solution);

Auction * parseAuction(const string & filename);


Clinics * parseClinics(const string & filename, const string & dir);
ClinicsTest * parseClinicsTest(const string & filename, int nresources);


void findChar(istream & str, char c);
void findStart(istream & str);
int readAssingnment(istream & str);
vector<int> readExecution(istream & str, vector<int> &is_exec);

}

#endif

