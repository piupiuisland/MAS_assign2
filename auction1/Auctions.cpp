#include "Auctions.h"

Auction::Auction()
{
	this->buyers.resize(NUMBER_BUYERS);
	this->sellers.resize(NUMBER_SELLERS);

	// initial the decrease factors and increase factors for each buyer
	initFactors(); 
	

}

void Auction::auctionSimulation()
{
	int order[NUMBER_SELLERS];
	std::ofstream outputfile;

	outputfile.open("myfile.txt"); 
	if (outputfile.fail()) {   
		perror("myfile.txt"); 
	}

	// output basic information
	output(outputfile);

	// Randomly assign items and starting prices in every round for every seller
	assignItem(outputfile);

	outputfile << "-------------------------------------------------------------------- " << std::endl;
	outputfile << "Pure auctions: " << std::endl;
	// initial the bidding factors for each buyer purchasing related item from each seller
	initBiddingFactors(outputfile);
	
	for (int r = 0; r < NUMBER_ROUND; r++) {
		outputfile << "Round " << r << std::endl;

		// each seller sets a starting price for their items
		setStartingPrice(outputfile);
		
		setOrder(outputfile, order); // !!!!!!!!!!!!!!!!!!!!!!!! everyloop the order will not change!!
		updateBid(outputfile);


		// Calculate the "pure" auctions
		pureAuction(outputfile, order);

		// update the bidding factors for each buyer
		updateBiddingFactor(outputfile, r);
		outputfile << "-------------------------------------------------------------------- " << std::endl;

	}

	outputfile << "Leveled commitment auctions: " << std::endl;
	/////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! think about the data gan rao problem
	// initial the bidding factors for each buyer purchasing related item from each seller
	initBiddingFactors(outputfile);

	for (int r = 0; r < NUMBER_ROUND; r++) {

		// each seller sets a starting price for their items
		setStartingPrice(outputfile);
		setOrder(outputfile, order);
		updateBid(outputfile);

		// Calculate the leveled commitment auctions
		LCAuction();

		// update the bidding factors for each buyer
		updateBiddingFactor(outputfile, r);
		outputfile << "-------------------------------------------------------------------- " << std::endl;

	}
	

	outputfile.close();


	
}

void Auction::assignItem(std::ofstream &outputfile)
{
	srand((unsigned)time(NULL));
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		this->sellers[k].item.itemSerial = rand() % NUMBER_ITEMS;
		outputfile << "In each round, seller " << k << " will sell item: "
			<< this->sellers[k].item.itemSerial << std::endl;
	}
	outputfile << std::endl;
}

void Auction::setStartingPrice(std::ofstream &outputfile)
{
	srand((unsigned)time(NULL));
	outputfile << "The starting price setted by each seller: " << std::endl;
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		// the starting price should be between 1 and "SMAX"
		this->sellers[k].item.setStartingPrice(rand() % (SMAX-1)+1);
		outputfile << "seller: " << k << "	"
			<< this->sellers[k].item.getStartingPrice() << std::endl;;
	}
	outputfile << std::endl;
}

void Auction::setOrder(std::ofstream &outputfile, int *order)
{
	srand(unsigned(time(NULL)));
	std::vector<int> myvector;
	
	for (int i = 0; i < NUMBER_SELLERS; ++i) myvector.push_back(i); 
	random_shuffle(myvector.begin(), myvector.end());
	for (int i = 0; i < NUMBER_SELLERS; ++i) order[i] = myvector[i];
	outputfile << "The order of the seller in this round is: ";
	for (int i = 0; i < NUMBER_SELLERS; ++i) outputfile << order[i] << "	";
	outputfile << std::endl << std::endl;

}

void Auction::pureAuction(std::ofstream &outputfile, int *order)
{
	double sum = 0;
	for (int k = 0; k < NUMBER_SELLERS; k++) {
		for (int n = 0; n < NUMBER_BUYERS; n++) {
			sum += buyers[n].bid[k];
			marketPrice[k] = sum / NUMBER_BUYERS;
			/////////////////////////////////////////// who is the winner, the function can output the buyer's serial number and the price he should pay
			


		}

	}






	for (int n = 0; n < NUMBER_BUYERS; n++) {
		for (int r = 0; r < NUMBER_ROUND; r++) {
			for (int k = 0; k < NUMBER_SELLERS; k++) {
				buyers[n].win[r][k] = true;
				// calcaulate the bool win
			}
		}
	}


}

void Auction::LCAuction()
{


	for (int n = 0; n < NUMBER_BUYERS; n++) {
		for (int r = 0; r < NUMBER_ROUND; r++) {
			for (int k = 0; k < NUMBER_SELLERS; k++) {
				buyers[n].win[r][k] = true;
				// calcaulate the bool win
			}
		}
	}

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
	srand((unsigned)time(NULL));
	outputfile << "The bidding factors:" << std::endl;
	//outputfile << "items sold by seller: ";
	//for (int k = 0; k < NUMBER_SELLERS; k++) {
	//	outputfile << "	" << k;
	//}
	outputfile << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer " << n << " : ";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			scope = (SMAX - this->sellers[k].item.getStartingPrice());
			this->buyers[n].biddingFactor[k] = 1.1; //!!!!!!!!!!!!!!!!!!!!!!!!!!!! waiting random data between 1 and scope/...
			outputfile << this->buyers[n].biddingFactor[k] << "	";
		}
		outputfile << std::endl << std::endl;
	}


}

void Auction::updateBiddingFactor(std::ofstream &outputfile, int round)
{
	outputfile << "The bidding factors after updated: " << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer " << n << " : ";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			if (buyers[n].win[round][k] == true) {
				buyers[n].biddingFactor[k] *= buyers[n].decreaseFactor;
			}
			else {
				buyers[n].biddingFactor[k] *= buyers[n].increaseFactor;
			}
			outputfile << this->buyers[n].biddingFactor[k] << "	";
		}
		outputfile << std::endl << std::endl << std::endl;
	}
	

}

void Auction::output(std::ofstream &outputfile)
{
	outputfile << "NUMBER_ITEMS: " << NUMBER_ITEMS << "	";
	outputfile << "NUMBER_SELLERS: " << NUMBER_SELLERS << "	";
	outputfile << "NUMBER_BUYERS: " << NUMBER_BUYERS << "	";
	outputfile << "Penalty factor: " << EPSILON << "	";
	outputfile << "SMAX: " << SMAX << "	" << std::endl;
	outputfile << "Bid decrease factor for each buyer: ";
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << buyers[n].decreaseFactor << "	";
	}
	outputfile << std::endl << std::endl;
	outputfile << "Bid increase factor for each buyer: ";
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		 outputfile << buyers[n].increaseFactor << "	";
	}
	outputfile << std::endl << std::endl;

}

void Auction::updateBid(std::ofstream &outputfile)
{
	outputfile << "The bid: " << std::endl;
	for (int n = 0; n < NUMBER_BUYERS; n++) {
		outputfile << "Buyer " << n << " : ";
		for (int k = 0; k < NUMBER_SELLERS; k++) {
			buyers[n].bid[k] = buyers[n].biddingFactor[k] * sellers[k].item.getStartingPrice();
			outputfile << buyers[n].bid[k] << "	";

		}
		outputfile << std::endl;
	}
	outputfile << std::endl;
}


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

Buyer::Buyer()
{
	
}
