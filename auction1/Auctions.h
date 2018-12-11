#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>


#define NUMBER_ITEMS 2
#define NUMBER_SELLERS 3
#define NUMBER_BUYERS 4
#define NUMBER_ROUND 5
#define EPSILON 0.08 // a penalty factor
#define SMAX 100 // universal maximum starting price
//#define AUCTION_TYRE "pure" // either "pure" or "LC"


// Single item
class Item {


public:
	void setStartingPrice(double stPri);
	double getStartingPrice();
	int itemSerial; // the serial number of this item is assigned with

private:
	double startingPrice;
};


 // Single buyer
class Buyer { 
public:
	Buyer();
public:
	double decreaseFactor;
	double increaseFactor;
	double bid[NUMBER_SELLERS]; // Bid of each buyer for sellers' item in a single round
	double biddingFactor[NUMBER_SELLERS]; // a unique bidding factor of this buyer puchasing the related item from each seller
	bool win[NUMBER_ROUND][NUMBER_SELLERS]; //!!!!!!!!!!!!!!!!!!!!!!! to be updated after each auction
private:
	
private:
	double profit = 0.;


};

// Single seller
class Seller {
public:
	Seller();
public:
	Item item; // the item this seller will sell in every round
private:
	
private:
	double profit = 0.;

};


class Auction
{
public:
	Auction();
	
	void auctionSimulation();
public:


private:
	void assignItem(std::ofstream &outputfile);
	void setStartingPrice(std::ofstream &outputfile);
	void setOrder(std::ofstream &outputfile, int *order);
	void pureAuction(std::ofstream &outputfile, int *order);
	void LCAuction();
	void initFactors();
	void initBiddingFactors(std::ofstream &outputfile);
	void updateBiddingFactor(std::ofstream &outputfile, int round);
	void output(std::ofstream &outputfile);
	void updateBid(std::ofstream &outputfile);

private:
	std::vector <Buyer> buyers;
	std::vector <Seller> sellers;
	double marketPrice[NUMBER_SELLERS];
	

	
};

