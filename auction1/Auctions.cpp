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
//
//void Auction::auctionSimulation()
//{
//	int order[NUMBER_SELLERS]; // the sellers' order in each round
//	std::ofstream outputfile;
//
//	outputfile.open("myfile.txt"); 
//	if (outputfile.fail()) {   
//		perror("myfile.txt"); 
//	}
//
//	// output basic information
//	outputBeforeSimulation(outputfile);
//
//	// Randomly assign items and starting prices in every round for every seller
//	assignItem(outputfile);
//
//	outputfile << "**************************** Pure Auctions ****************************" << std::endl;
//
//	// initial the bidding factors for each buyer purchasing related item from each seller
//	initBiddingFactors(outputfile);
//	
//	// each seller sets a starting price for their items
//	setStartingPrice(outputfile);
//
//	//pure auction
//	for (int r = 0; r < NUMBER_ROUND; r++) {
//		outputfile << "----------------------- Round " << r << " -----------------------" << std::endl;
//		
//		setOrder(outputfile, order); // !!!!!!!!!!!!!!!!!!!!!!!!in everyloop the order will not change!!
//		updateBid(outputfile,order);
//
//		// Calculate the "pure" auctions
//		pureAuction(outputfile, order, r);
//
//		// update bidding factors after each round
//		updateBiddingFactor(outputfile, order, r);
//	}
//
//	outputfile << "**************************** Leveled Commitment Auctions ****************************" << std::endl;
//	
//	// initial all the values
//	initialValues();
//	
//	initBiddingFactors(outputfile);
//	// LC auction
//	for (int r = 0; r < NUMBER_ROUND; r++) {
//		outputfile << "----------------------- Round " << r << " -----------------------" << std::endl;
//
//		setOrder(outputfile, order);
//		updateBid(outputfile, order);
//
//		LCAuction(outputfile, order, r);
//
//		updateBiddingFactor(outputfile, order, r);
//	}
//	outputfile.close();
//}

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
		sellers[thisAuction].setProfit(marketPrice[thisAuction]);
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
			if (winBefore(n, round, thisAuction)) {
				sum += maxBid(n, thisAuction, round);
				buyers[n].bid[thisAuction] = maxBid(n, thisAuction, round);
			}
			else sum += buyers[n].bid[thisAuction];
		}

		marketPrice[thisAuction] = sum / double(NUMBER_BUYERS);
		getWinner(k,order, false, round); // assign winner's serial No. and his bid
		


		// update the seller's profit in this auction
		sellers[thisAuction].setProfit(marketPrice[thisAuction]);
		std::cout << sellers[thisAuction].getProfit() << std::endl;
		
		// if the winner has win in previous auctions, the he need to pay the penalty
		int previousAuction;
		if (winBefore(winBuyer[thisAuction], round,thisAuction)) {
			for (int kk = 0; kk < k; kk++) {
				if (buyers[winBuyer[thisAuction]].win[round][order[kk]] == true) previousAuction = order[kk];
				buyers[winBuyer[thisAuction]].win[round][kk] = false;
			}
			// change seller's profit in the previous auction
			sellers[previousAuction].setProfit(-marketPrice[previousAuction]+ EPSILON* winBuyerBid[previousAuction]);
			
			//update the winner buyer's profit
			buyers[winBuyer[thisAuction]].setProfit(
				marketPrice[thisAuction] - winBuyerBid[thisAuction] 
				-(marketPrice[previousAuction] - winBuyerBid[previousAuction])
				- EPSILON * winBuyerBid[previousAuction]);
		}
		else {
			buyers[winBuyer[thisAuction]].setProfit(marketPrice[thisAuction] - winBuyerBid[thisAuction]);
			sellers[thisAuction].setSecondProfit(marketPrice[thisAuction]);
			buyers[winBuyer[thisAuction]].setSecondProfit(buyers[winBuyer[thisAuction]].getProfit());

			std::cout << buyers[winBuyer[thisAuction]].getProfit() << std::endl;
		}
	}
	outputAfterSimulation(outputfile, order);
}

void Auction::initFactors()
{
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		this->buyers[n].decreaseFactor = 0.09;
		this->buyers[n].increaseFactor = 1.01;
	}
}

void Auction::initBiddingFactors(std::ofstream &outputfile)
{
	double scope; 
	//srand((unsigned)time(NULL)); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!random should not changed each simulation
	outputfile << "Initial bidding factors:" << std::endl;
	
	outputfile << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer" << n << ":	";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			scope = (SMAX - this->sellers[k].item.getStartingPrice());
			this->buyers[n].biddingFactor[k] = double(RAND_MAX)/(rand()+1); // random factors larger than one
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
	int thisAuction;
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

	outputfile << std::endl << "Winbuyers' bid: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << winBuyerBid[thisAuction] << "	";
	}

	outputfile << std::endl << "Winbuyers' profits: " << "	";
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		thisAuction = order[k];
		outputfile << buyers[winBuyer[thisAuction]].getProfit() << "	";
	}
	//outputfile << std::endl << "Other buyers' profits didn't change.";

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
		if (winBuyerBid[thisAuction] == 0) winBuyerBid[thisAuction] = firstPrice;
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

	buyers[winBuyer[thisAuction]].win[round][thisAuction] = true; // update the "win" variable
}

double Auction::maxBid(int n, int k, int round)
{
	int winSerial;
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		if (buyers[n].win[round][k] == true) {
			winSerial = k;
		}
	}
	
	double max = sellers[k].item.getStartingPrice() +
		marketPrice[winSerial] - buyers[n].bid[winSerial] + EPSILON* buyers[n].bid[winSerial];
		
	if (buyers[n].bid[k] > max) return buyers[n].bid[k];
	else return max;
}

bool Auction::winBefore(int n, int round, int thisAuction) // if buyer "n" has won in previous auctions
{
	bool buyerWin = false;
	for (int k = 0; k < NUMBER_SELLERS; k++)
		if (k != thisAuction)
			if (buyers[n].win[round][k] == true) buyerWin = true;
	if (buyerWin) return true;
	else return false;
}

//void Auction::initialValues()
//{
//	for (int n = 0; n < NUMBER_BUYERS; n++) {
//		buyers[n].setProfit(0.);
//		for (int r = 0; r < NUMBER_ROUND; r++) {
//			for (int k = 0; k < NUMBER_SELLERS; k++) {
//				buyers[n].win[r][k] = false;
//			}
//		}
//	}
//	for (int k = 0; k < NUMBER_SELLERS; k++) {
//		sellers[k].setProfit(0.);
//		winBuyer[k] = 0;
//		winBuyerBid[k] = 0.;
//		marketPrice[k] = 0.;
//	}
//	
//
//}


void Item::setStartingPrice(double stPri)
{
	this->startingPrice = stPri;
}

double Item::getStartingPrice()
{
	return this->startingPrice;
}

Seller::Seller()
{
	
	
}

void Seller::setProfit(double prof)
{
	this->profit += prof;
}

double Seller::getProfit()
{
	return this->profit;
}

void Seller::setSecondProfit(double prof)
{
	this->secondProfit = prof;
}

double Seller::getSecondProfit()
{
	return this->secondProfit;
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

void Buyer::setSecondProfit(double prof)
{
	this->secondProfit = prof;
}

double Buyer::getSecondProfit()
{
	return this->secondProfit;
}

void PureAuction::Simulation()
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

	for (int r = 0; r < NUMBER_ROUND; r++) {
		outputfile << "----------------------- Round " << r << " -----------------------" << std::endl;

		Auction.setOrder(outputfile, order); 
		Auction.updateBid(outputfile, order);
		Auction.outputBid(outputfile, order);

		// Calculate the "pure" auctions
		Auction.pureAuction(outputfile, order, r);

		// update bidding factors after each round
		Auction.updateBiddingFactor(outputfile, order, r);
	}
	outputfile.close();
}

void LCAuction::Simulation()
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
	for (int r = 0; r < NUMBER_ROUND; r++) {
		outputfile << "----------------------- Round " << r << " -----------------------" << std::endl;

		Auction.setOrder(outputfile, order);
		Auction.updateBid(outputfile, order);
		Auction.LCAuction(outputfile, order, r);
		Auction.updateBiddingFactor(outputfile, order, r);
	}
	outputfile.close();
}
