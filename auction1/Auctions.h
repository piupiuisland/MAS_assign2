#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>

#define NUMBER_ITEMS 2
#define NUMBER_SELLERS 3
#define NUMBER_BUYERS 4 // Attention: Number of buyers should larger than number of sellers!
#define NUMBER_ROUND 5
#define EPSILON 0.08 // a penalty factor
#define SMAX 100 // universal maximum starting price

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
	void setProfit(double prof);
	double getProfit();
public:
	double decreaseFactor;
	double increaseFactor;
	double bid[NUMBER_SELLERS]; // Bid of each buyer for sellers' item in a single round
	double biddingFactor[NUMBER_SELLERS]; // a unique bidding factor of this buyer puchasing the related item from each seller
	bool win[NUMBER_ROUND][NUMBER_SELLERS]; // whether this buyer wins or not in all rounds
	
private:
	double profit = 0.;
};

// Single seller
class Seller {
public:
	Seller();
	void setProfit(double prof);
	double getProfit();
public:
	Item item; // the item this seller will sell in every round
	
private:
	double profit = 0.;
};


class Auction
{
public:
	Auction();

	void assignItem(std::ofstream &outputfile);
	void setStartingPrice(std::ofstream &outputfile);
	void setOrder(std::ofstream &outputfile, int *order);
	void pureAuction(std::ofstream &outputfile, int *order, int round);
	void LCAuction(std::ofstream &outputfile, int *order, int round);
	void initFactors();
	void initBiddingFactors(std::ofstream &outputfile);
	void updateBiddingFactor(std::ofstream &outputfile, int *order, int round);
	void outputBeforeSimulation(std::ofstream &outputfile);
	void outputAfterSimulation(std::ofstream &outputfile, int *order);
	void updateBid(std::ofstream &outputfile, int *order);
	void outputBid(std::ofstream &outputfile, int *order);
	void getWinner(int k, int *order, bool pure, int round);
	double maxBid(int n, int k, int round);
	bool winBefore(int n, int round, int thisAuction);

public:
	std::vector <Buyer> buyers;
	std::vector <Seller> sellers;
	double marketPrice[NUMBER_SELLERS];
	int winBuyer[NUMBER_SELLERS];// the serial number of buyer winner in each auction
	double winBuyerBid[NUMBER_SELLERS]; // the winner's bid in each auction
};

class PureAuction {
public:
	void Simulation();
private:
	Auction Auction;
};

class LCAuction {
public:
	void Simulation();
private:
	Auction Auction;
};
