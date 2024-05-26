#ifndef AUCTION_DEFINITION
#define AUCTION_DEFINITION
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <set>
using namespace std;

class Auction
{
  int nitems; //number of items
  int nbids; //number of bids
  int * bid_v; //value of the bids
  set<int> * bids; //items in each bid


 public:
    Auction(int nitems, int nbids);
    int getNItems() const;
    int getNBids() const;

	const set<int> getBid(int bid) const;

	void addItem(int bid, int item);
	bool demandsItem(int bid, int item) const;

	void setBidValue(int bid, int value);
	int getBidValue(int bid) const;

	bool shareItem(int b1, int b2) const;

	int computeLB() const;
	int computeUB() const;

	void computeBidCover(vector<vector<int> > & cover) const;


    friend ostream &operator <<(ostream &, Auction &);

	 ~Auction();
};

#endif

