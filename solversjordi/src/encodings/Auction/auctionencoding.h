#ifndef AUCTIONENCODING_DEFINITION
#define AUCTIONENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "encoding.h"
#include "auction.h"

using namespace std;
using namespace smtapi;

class AuctionEncoding : public Encoding {

private:

	Auction * instance;
	AMOPBEncoding amopbenc;

	vector<bool> sold;

public:

	AuctionEncoding(Auction * instance, AMOPBEncoding amopbenc);
	~AuctionEncoding();

	SMTFormula *  encode(int lb = INT_MIN, int ub = INT_MAX);

	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);

	int getObjective() const;

	bool printSolution(ostream &os) const;

};

#endif

