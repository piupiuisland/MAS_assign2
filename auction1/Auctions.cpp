#include "Auctions.h"

Auction::Auction()
{
	this->buyers.resize(NUMBER_BUYERS);
	this->sellers.resize(NUMBER_SELLERS);

	// initial the decrease factors and increase factors for each buyer
	initFactors(); 

	// initial "winBuyer" array
	for (int k = 0; k < NUMBER_SELLERS; k++)
		winBuyer[k] = std::numeric_limits<int>::max();
}

void Auction::assignItem(std::ofstream &outputfile)
{
	//srand((unsigned)time(NULL));
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		this->sellers[k].item.itemSerial = rand() % NUMBER_ITEMS;
		outputfile << "Seller " << k << " will sell item: "
			<< this->sellers[k].item.itemSerial << std::endl;
	}
	outputfile << std::endl;
}

void Auction::setStartingPrice(std::ofstream &outputfile)
{
	//srand((unsigned)time(NULL));
	outputfile << "The starting prices: " << std::endl;

	for (int k = 0; k < NUMBER_SELLERS; k++) {

		// the starting price should be between 1 and "SMAX"
		this->sellers[k].item.setStartingPrice(rand() % (SMAX-1)+1);
		
		outputfile << "seller" << k <<":"<< "	"
			<< this->sellers[k].item.getStartingPrice() << std::endl;;
	}
	outputfile << std::endl;
}

void Auction::setOrder(std::ofstream &outputfile, int *order)
{
	//srand(unsigned(time(NULL)));
	std::vector<int> myvector;
	
	for (int i = 0; i < NUMBER_SELLERS; ++i) myvector.push_back(i); 
	random_shuffle(myvector.begin(), myvector.end());
	for (int i = 0; i < NUMBER_SELLERS; ++i) order[i] = myvector[i];
	outputfile << "The order of the seller in this round is:	";
	for (int i = 0; i < NUMBER_SELLERS; ++i) outputfile << order[i] << "	";
	outputfile << std::endl << std::endl;
}

void Auction::pureAuction(std::ofstream &outputfile, int *order, int round)
{
	double sum = 0;
	int thisAuction;
	for (int k = 0; k < NUMBER_SELLERS; k++) { 
		thisAuction = order[k];
		sum = 0.;
		for (int n = 0; n < NUMBER_BUYERS; n++) 
			sum += buyers[n].bid[thisAuction];

		// delete the buyers who have won in previous auctions
		for(int kk = 0; kk < k; kk++)
			sum -= buyers[winBuyer[order[kk]]].bid[thisAuction];

		// calculate market price and winner buyer
		marketPrice[thisAuction] = sum / double(NUMBER_BUYERS-k);
		getWinner(k, order, true,round); // assign winner's serial No. and his bid
		
		// update the seller's and winner buyer's profits
		sellers[thisAuction].setProfit(winBuyerBid[thisAuction]);
		buyers[winBuyer[thisAuction]].setProfit(marketPrice[thisAuction] - winBuyerBid[thisAuction]);
	
	}
	outputAfterSimulation(outputfile,order);
}

void Auction::LCAuction(std::ofstream &outputfile, int *order, int round)
{
	double sum;
	int thisAuction;
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		sum = 0.;
		// get the sum of all the buyers' bid depending whether this buyer has won before
		for (int n = 0; n < NUMBER_BUYERS; n++) {
			if (winBefore(n, round, order, k)) {
				sum += maxBid(n, thisAuction, round);
				buyers[n].bid[thisAuction] = maxBid(n, thisAuction, round);
			}
			else sum += buyers[n].bid[thisAuction];
		}

		marketPrice[thisAuction] = sum / double(NUMBER_BUYERS);
		getWinner(k,order, false, round); // assign winner's serial No. and his bid

		// update the seller's profit in this auction
		sellers[thisAuction].setProfit(winBuyerBid[thisAuction]);
		//std::cout << sellers[thisAuction].getProfit() << std::endl;
		
		// if the winner has win in previous auctions, the he need to pay the penalty
		int previousAuction;
		if (winBefore(winBuyer[thisAuction], round, order, k)) {
			for (int kk = 0; kk < k; kk++) {
				if (buyers[winBuyer[thisAuction]].win[round][order[kk]] == true) previousAuction = order[kk];
			}
			// change seller's profit in the previous auction
			sellers[previousAuction].setProfit(-winBuyerBid[previousAuction]+ EPSILON* winBuyerBid[previousAuction]);
			
			//update the winner buyer's profit
			buyers[winBuyer[thisAuction]].setProfit(
				marketPrice[thisAuction] - winBuyerBid[thisAuction] 
				-(marketPrice[previousAuction] - winBuyerBid[previousAuction])
				- EPSILON * winBuyerBid[previousAuction]);
		}
		else {
			buyers[winBuyer[thisAuction]].setProfit(marketPrice[thisAuction] - winBuyerBid[thisAuction]);
			sellers[thisAuction].setPreviousProfit(sellers[thisAuction].getProfit());
			buyers[winBuyer[thisAuction]].setPreviousProfit(buyers[winBuyer[thisAuction]].getProfit());

			//std::cout << buyers[winBuyer[thisAuction]].getProfit() << std::endl;
		}
	}
	outputAfterSimulation(outputfile, order);
}

void Auction::initFactors()
{
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		this->buyers[n].decreaseFactor = 0.9;
		this->buyers[n].increaseFactor = 1.1;
	}
}

void Auction::initBiddingFactors(std::ofstream &outputfile)
{
	double scope; 
	outputfile << "Initial bidding factors:" << std::endl;

	//srand((unsigned)time(NULL));
	outputfile << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer" << n << ":	";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			scope = (SMAX - this->sellers[k].item.getStartingPrice());
			this->buyers[n].biddingFactor[k] = 1 + (UP_BOUNDART - 1)*rand() / double(RAND_MAX); // random factors between (1, UP_BOUNDERT)
			
			outputfile << this->buyers[n].biddingFactor[k] << "	";
		}
		outputfile << std::endl;
	}
	outputfile << std::endl;
}

void Auction::updateBiddingFactor(std::ofstream &outputfile, int *order, int round)
{
	int thisAuction;
	outputfile << "The bidding factors after updated:" << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer" << n << ":	";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			thisAuction = order[k];
			if (buyers[n].win[round][thisAuction] == true) {
				buyers[n].biddingFactor[thisAuction] *= buyers[n].decreaseFactor;
				if (buyers[n].biddingFactor[thisAuction] < 1.)
					buyers[n].biddingFactor[thisAuction] = 1.;
			}
			else {
				buyers[n].biddingFactor[thisAuction] *= buyers[n].increaseFactor;
			}
			outputfile << this->buyers[n].biddingFactor[thisAuction] << "	";
		}
		outputfile << std::endl;
	}
	outputfile << std::endl;

}

void Auction::outputBeforeSimulation(std::ofstream &outputfile)
{
	outputfile << "NUMBER_ITEMS: " << NUMBER_ITEMS << "	";
	outputfile << "NUMBER_SELLERS: " << NUMBER_SELLERS << "	";
	outputfile << "NUMBER_BUYERS: " << NUMBER_BUYERS << "	";
	outputfile << "NUMBER_ROUND: " << NUMBER_ROUND << "	";
	outputfile << "Penalty factor: " << EPSILON << "	";
	outputfile << "UP_BOUNDART: " << UP_BOUNDART << "	";
	outputfile << "SMAX: " << SMAX << "	" << std::endl << std::endl;
	outputfile << "Bid decrease factor for each buyer: ";
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << buyers[n].decreaseFactor << "	";
	}
	outputfile << std::endl;
	outputfile << "Bid increase factor for each buyer: ";
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		 outputfile << buyers[n].increaseFactor << "	";
	}
	outputfile << std::endl << std::endl;

}

void Auction::outputAfterSimulation(std::ofstream & outputfile, int *order)
{
	int thisAuction, thisAuctionkk, winSerial;
	bool winAfter = false;
	bool win_After = false;
	outputBid(outputfile, order);

	outputfile << std::endl << "Market prices: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << marketPrice[thisAuction] << "	";
	}

	outputfile << std::endl << "Winbuyers: " <<"	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << winBuyer[thisAuction] << "	";
	}

	outputfile << std::endl << "Winbuyers' orginal bids: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << buyers[winBuyer[thisAuction]].bid[thisAuction] << "	";
	}

	outputfile << std::endl << "Winbuyers' bids: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << winBuyerBid[thisAuction] << "	";
	}

	outputfile << std::endl << "Winbuyers' profits: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		
		// if this win buyer has win twice, then output the precious profit in the first time
		// and output the current profit for the second time
		for (int kk = k + 1; kk < NUMBER_SELLERS; kk++) {
			thisAuctionkk = order[kk];
			if (winBuyer[thisAuction] == winBuyer[thisAuctionkk]) winAfter = true;
		}
		if (winAfter) outputfile << buyers[winBuyer[thisAuction]].getPreviousProfit() << "	";
		else outputfile << buyers[winBuyer[thisAuction]].getProfit() << "	";
		winAfter = false;
	}

	outputfile << std::endl << "Sellers' profits: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << sellers[thisAuction].getProfit() << "	";
	}

	outputfile << std::endl << std::endl;
}

void Auction::updateBid(std::ofstream &outputfile, int *order)
{
	int thisAuction;
	outputfile << "The biding factors:	" << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer" << n << ":	";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			thisAuction = order[k];
			outputfile << buyers[n].biddingFactor[thisAuction] << "	";
		}
		outputfile << std::endl;
	}
	outputfile << std::endl;

	for (int n = 0; n < NUMBER_BUYERS; n++) {
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			thisAuction = order[k];
			buyers[n].bid[thisAuction] = buyers[n].biddingFactor[thisAuction] * sellers[thisAuction].item.getStartingPrice();
		}
	}
}

void Auction::outputBid(std::ofstream & outputfile, int * order)
{
	int thisAuction;
	outputfile << "The bids: " << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer" << n << ":	";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			thisAuction = order[k];
			outputfile << buyers[n].bid[thisAuction] << "	";
		}
		outputfile << std::endl;
	}
	outputfile << std::endl;
}


void Auction::getWinner(int k, int *order, bool pure, int round)
{
	double firstPrice = 0., secondPrice = 0.;
	int thisAuction = order[k];
	bool winPrevious = false;
	if (pure) {

		for (int n = 0; n < NUMBER_BUYERS; n++) {
			for (int kk = 0; kk < k; kk++)
				if (n == winBuyer[order[kk]]) winPrevious = true;
			if (not winPrevious) {

				if (buyers[n].bid[thisAuction] <= marketPrice[thisAuction]) {
					if (buyers[n].bid[thisAuction] >= firstPrice) {
						secondPrice = firstPrice;
						firstPrice = buyers[n].bid[thisAuction];
						winBuyer[thisAuction] = n;
						winBuyerBid[thisAuction] = secondPrice;
					}
					else if (buyers[n].bid[thisAuction] < firstPrice && buyers[n].bid[thisAuction] > secondPrice) {
						secondPrice = buyers[n].bid[thisAuction];
						winBuyerBid[thisAuction] = secondPrice;
					}
				}
			}
			winPrevious = false;
		}
	}
	else {
		for (int n = 0; n < NUMBER_BUYERS; n++) {
			if (buyers[n].bid[thisAuction] <= marketPrice[thisAuction]) {
				if (buyers[n].bid[thisAuction] >= firstPrice) {
					secondPrice = firstPrice;
					firstPrice = buyers[n].bid[thisAuction];
					winBuyer[thisAuction] = n;
					winBuyerBid[thisAuction] = secondPrice;
				}
				else if (buyers[n].bid[thisAuction] < firstPrice && buyers[n].bid[thisAuction] > secondPrice) {
					secondPrice = buyers[n].bid[thisAuction];
					winBuyerBid[thisAuction] = secondPrice;
				}
			}
		}
	}
	if (winBuyerBid[thisAuction] == 0) winBuyerBid[thisAuction] = firstPrice;
	buyers[winBuyer[thisAuction]].win[round][thisAuction] = true; // update the "win" variable
}

double Auction::maxBid(int n, int k, int round)
{
	int winSerial;
	for (int kk = 0; kk < NUMBER_SELLERS; kk++) {
		if (buyers[n].win[round][kk] == true) {
			winSerial = kk;
		}
	}
	
	double max = sellers[k].item.getStartingPrice() +
		marketPrice[winSerial] - buyers[n].bid[winSerial] + EPSILON* buyers[n].bid[winSerial];
		
	if (buyers[n].bid[k] > max) return buyers[n].bid[k];
	else return max;
}

bool Auction::winBefore(int n, int round, int *order, int k) // if buyer "n" has won in previous auctions
{
	bool buyerWin = false;
	int thisAuction;
	for (int kk = 0; kk < k; kk++) {
		thisAuction = order[kk];
		if (buyers[n].win[round][thisAuction] == true) buyerWin = true;
	}
	if (buyerWin) return true;
	else return false;
}


void Item::setStartingPrice(double stPri)
{
	this->startingPrice = stPri;
}

double Item::getStartingPrice()
{
	return this->startingPrice;
}

void Seller::setProfit(double prof)
{
	this->profit += prof;
}

double Seller::getProfit()
{
	return this->profit;
}

void Seller::setPreviousProfit(double prof)
{
	this->previousProfit = prof;
}

double Seller::getPreviousProfit()
{
	return this->previousProfit;
}

Buyer::Buyer()
{
	for (int round = 0; round < NUMBER_ROUND; round++) {
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			win[round][k] = false; // initial "lose" to every buyer
		}
	}
}

void Buyer::setProfit(double prof)
{
	this->profit += prof;
}

double Buyer::getProfit()
{
	return this->profit;
}

void Buyer::setPreviousProfit(double prof)
{
	this->previousProfit = prof;
}

double Buyer::getPreviousProfit()
{
	return this->previousProfit;
}

void PureAuction::simulation()
{
	int order[NUMBER_SELLERS]; // the sellers' order in each round
	std::ofstream outputfile;

	outputfile.open("PureAuction.txt");
	if (outputfile.fail()) {
		perror("PureAuction.txt");
	}

	// output basic information
	Auction.outputBeforeSimulation(outputfile);

	// Randomly assign items and starting prices in every round for every seller
	Auction.assignItem(outputfile);

	outputfile << "**************************** Pure Auctions ****************************" << std::endl;

	// initial the bidding factors for each buyer purchasing related item from each seller
	Auction.initBiddingFactors(outputfile);

	// each seller sets a starting price for their items
	Auction.setStartingPrice(outputfile);

	//// output pureAuction initial bidding factors
	std::ofstream PureInitBidFactor;
	PureInitBidFactor.open("Pure_init_biddingfactor.txt");
	if (PureInitBidFactor.fail()) {
		perror("Pure_init_biddingfactor.txt");
	}
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			PureInitBidFactor << Auction.buyers[n].biddingFactor[k] << "	";
		}
		PureInitBidFactor << std::endl;
	}
	PureInitBidFactor.close();

	//output seller/buyer profits
	std::ofstream PureSeller;
	std::ofstream PureBuyer;
	PureSeller.open("Pure_seller_profit.txt");
	if (PureSeller.fail()) {
		perror("Pure_seller_profit.txt");
	}
	PureBuyer.open("Pure_buyer_profit.txt");
	if (PureBuyer.fail()) {
		perror("Pure_buyer_profit.txt");
	}////

	for (int r = 0; r < NUMBER_ROUND; r++) {
		outputfile << "----------------------- Round " << r+1 << " -----------------------" << std::endl;

		Auction.setOrder(outputfile, order); 
		Auction.updateBid(outputfile, order);
		//Auction.outputBid(outputfile, order);

		// Calculate the "pure" auctions
		Auction.pureAuction(outputfile, order, r);

		//// output sellers and buyers's profit
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			PureSeller << Auction.sellers[k].getProfit() << "	";
		}
		for (int n = 0; n < NUMBER_BUYERS; n++) {
			PureBuyer << Auction.buyers[n].getProfit() << "	";
		}
		PureSeller << std::endl;
		PureBuyer << std::endl;
		////

		// update bidding factors after each round
		Auction.updateBiddingFactor(outputfile, order, r);

	}

	outputfile << std::endl << "*************************** Final Results ***************************";
	outputfile << std::endl << std::endl << "All Sellers' profits: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		outputfile << Auction.sellers[k].getProfit() << "	";
	}

	outputfile << std::endl << "All buyers' profits: " << "	";
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << Auction.buyers[n].getProfit() << "	";
	}
	outputfile << std::endl;
	outputfile.close();
	PureSeller.close();////
	PureBuyer.close();////
}

void LCAuction::simulation()
{
	int order[NUMBER_SELLERS]; 
	std::ofstream outputfile;

	outputfile.open("LCAuction.txt");
	if (outputfile.fail()) {
		perror("LCAuction.txt");
	}

	Auction.outputBeforeSimulation(outputfile);
	Auction.assignItem(outputfile);

	outputfile << "**************************** Leveled Commitment Auctions ****************************" << std::endl;
	
	Auction.initBiddingFactors(outputfile);
	Auction.setStartingPrice(outputfile);

	//// output LCAuction initial bidding factors
	std::ofstream LCInitBidFactors;
	LCInitBidFactors.open("LC_init_biddingfactor.txt");
	if (LCInitBidFactors.fail()) {
		perror("LC_init_biddingfactor.txt");
	}
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			LCInitBidFactors << Auction.buyers[n].biddingFactor[k] << "	";
		}
		LCInitBidFactors << std::endl;
	}
	LCInitBidFactors.close();

	//output seller/buyer profits
	std::ofstream LCSeller;
	std::ofstream LCBuyer;
	LCSeller.open("LC_seller_profit.txt");
	if (LCSeller.fail()) {
		perror("LC_seller_profit.txt");
	}
	LCBuyer.open("LC_buyer_profit.txt");
	if (LCBuyer.fail()) {
		perror("LC_buyer_profit.txt");
	}
	////

	for (int r = 0; r < NUMBER_ROUND; r++) {
		outputfile << "----------------------- Round " << r+1 << " -----------------------" << std::endl;

		Auction.setOrder(outputfile, order);
		Auction.updateBid(outputfile, order);
		Auction.LCAuction(outputfile, order, r);

		//// output sellers and buyers's profit
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			LCSeller << Auction.sellers[k].getProfit() << "	";
		}
		for (int n = 0; n < NUMBER_BUYERS; n++) {
			LCBuyer << Auction.buyers[n].getProfit() << "	";
		}
		LCSeller << std::endl;
		LCBuyer << std::endl;
		////

		Auction.updateBiddingFactor(outputfile, order, r);
	}

	outputfile << std::endl << "*************************** Final Results ***************************";
	outputfile << std::endl << std::endl << "All Sellers' profits: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		outputfile << Auction.sellers[k].getProfit() << "	";
	}

	outputfile << std::endl << "All buyers' profits: " << "	";
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << Auction.buyers[n].getProfit() << "	";
	}

	outputfile.close();
	LCSeller.close();////
	LCBuyer.close();////
}
