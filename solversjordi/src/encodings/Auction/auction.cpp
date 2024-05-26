#include "limits.h"
#include "auction.h"

using namespace std;


  int nitems; //number of items
  int nbids; //number of bids
  int * bid_v; //value of the bids
  set<int> * bids; //items in each bid


Auction::Auction(int nitems, int nbids){
	this->nitems = nitems;
	this->nbids = nbids;
	bid_v = new int[nbids];
	bids = new set<int>[nbids];

}

Auction::~Auction(){
	delete [] bid_v;
	delete [] bids;
}

int Auction::getNItems() const{
	return nitems;
}

int Auction::getNBids() const{
	return nbids;
}

const set<int> Auction::getBid(int bid) const{
	return bids[bid];
}

void Auction::addItem(int bid, int item){
	bids[bid].insert(item);
}

bool Auction::demandsItem(int bid, int item) const{
	return bids[bid].find(item) != bids[bid].end();
}

void Auction::setBidValue(int bid, int value){
	bid_v[bid] = value;
}

int Auction::getBidValue(int bid) const{
	return bid_v[bid];
}

bool Auction::shareItem(int b1, int b2) const{
	for(int i1 : bids[b1])
		for(int i2 : bids[b2])
			if(i1==i2)
				return true;
	return false;
}

int Auction::computeLB() const{

	vector<int> solution;

	for(int i = 0; i < nbids; i++){
		bool possible = true;
		for(int j : solution){
			if(shareItem(i,j)){
				possible = false;
				break;
			}
		}
		if(possible)
			solution.push_back(i);
	}

	int lb = 0;

	for(int i : solution)
		lb += bid_v[i];

	return lb;
}

int Auction::computeUB() const{
	int sum = 0;
	for(int i = 0; i < nbids; i++)
		sum += bid_v[i];
	return sum;
}

void Auction::computeBidCover(vector<vector<int> > & cover) const {

	for(int i = 0; i < nbids; i++){
		bool added = false;
		for(vector<int> & v : cover){
			bool possible = true;
			for(int j : v){
				if(!shareItem(i,j)){
					possible = false;
					break;
				}
			}
			if(possible){
				v.push_back(i);
				added = true;
				break;
			}
		}
		if(!added)
			cover.push_back(vector<int>(1,i));
	}

}


ostream &operator <<(ostream & os, Auction & a){
	os << a.nbids << " " << a.nitems << endl;
	for(int i = 0; i < a.nbids; i++){
		for(int j : a.bids[i])
			os << j << " ";
		os << endl;
	}
	for(int i = 0; i < a.nbids; i++)
		os << a.bid_v[i] << " ";
	os << endl;
}


