#include "parser.h"
#include "basiccontroller.h"
#include "auctionencoding.h"
#include "solvingarguments.h"

using namespace std;
using namespace util;


int main(int argc, char **argv) {

	Arguments<int> * pargs
	= new Arguments<int>(
	//Program arguments
	{
	arguments::arg("filename","Instance file path.")
	},1,{},"Solve the combinatorial auctions problem.");

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	Auction * instance = parser::parseAuction(pargs->getArgument(0));
	AuctionEncoding * encoding = new AuctionEncoding(instance,
	sargs->getAMOPBEncoding());
	BasicController c(sargs,encoding,false,instance->computeLB(),instance->computeUB());
	c.run();

	return 0;
}

