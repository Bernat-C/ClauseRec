#include <string>
#include <vector>
#include <set>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <list>
#include <cmath>
#include <map>
#include "errors.h"
#include "util.h"
#include "yices2apiencoder.h"
#include "testencoding.h"
#include "parser.h"
#include "amopbmddbuilder.h"
#include "solvingarguments.h"
#include "basiccontroller.h"

using namespace std;

int main(int argc, char **argv) {


	TestEncoding * encoding = new TestEncoding("");
	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,arguments::nullProgArgs());
	BasicController c(sargs,encoding,false,0,0);
	c.run();
/*
	#ifdef TMPFILESPATH
		cout << TMPFILESPATH  << endl;
	#else
		cout << "indefinit" << endl;
	#endif*/
//		MMKP * mmkp = parser::parseMMKP("/users/pfc/jordic/scratch/instances/mmkp/Gwendal/10-5-5-G-CL-S/rp_hep_hoc_strong_46_0.data");
//		cout << *mmkp << endl;
//

// 	MRCPSP * m = parser::parseMRCPSP("/users/pfc/jordic/scratch/instances/mrcpsp/Boctor50/boct1.prb");
//
// 	cout << *m << endl;
/*
	vector<vector<int> > Q(2);
	vector<vector<literal> > X(2,vector<literal>(3));

	Q[0].push_back(1);
	Q[0].push_back(2);
	Q[0].push_back(3);
//	Q[0].push_back(5);

	Q[1].push_back(1);
	Q[1].push_back(2);
	Q[1].push_back(3);
//	Q[1].push_back(5);

//	Q[2].push_back(1);
//	Q[2].push_back(3);
//	Q[2].push_back(5);
//	Q[2].push_back(6);
//
//	Q[3].push_back(1);
//	Q[3].push_back(3);
//	Q[3].push_back(4);
//	Q[3].push_back(5);

	AMOPBMDDBuilder builder(Q,X,4);
	builder.getMDD();

	ofstream os("MDD1.dot");
	builder.createGraphviz(os);
	os.close();

	builder.addRoot(3);
	ofstream os2("MDD2.dot");
	builder.createGraphviz(os2);
	os2.close();

	builder.addRoot(1);
	ofstream os3("MDD3.dot");
	builder.createGraphviz(os3);
	os3.close();

	builder.addRoot(5);
	ofstream os4("MDD4.dot");
	builder.createGraphviz(os4);
	os4.close();

	builder.addRoot(-1);
	ofstream os5("MDD5.dot");
	builder.createGraphviz(os5);
	os5.close();
*/
	return 0;
}

