#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <list>

using namespace std;

struct TreeNode {
	int elementValue;
	int elementFrequency;
	//TreeNode* down;
	TreeNode* up;
	list<TreeNode*> down;
	TreeNode* nextSimilar;
};

class SRPTree {
	int rareMinSup;
	int freqMinSup;
	int windowSize; //number of transactions after which mining should start
	int inputWindowSize; //input window size given by a user
	int distinctElements;
	int inputDistinctElements;
	string filename;
	ifstream in;
	TreeNode *rootNode;
	
	vector<int> dbElementFrequency; //Vector created to store the frequency of each element
	string sTransaction;  //One Transaction information in string
	list<int> iTransaction;  //List of integers for each transaction

	void ExtractIntegersToList();
	void AddElementFrequency();
	void AddToTree();

public:
	SRPTree();

	int Initialize();
	int Finalize();
	int ReadTransaction();
	int GetWindowSize();
	void Mine();
	//void GetFreqMinSup();
};
