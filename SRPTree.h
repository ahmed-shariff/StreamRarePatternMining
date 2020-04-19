#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <list>
#include <map>

using namespace std;

struct TreeNode {
	int elementValue;
	int elementFrequency;
	//TreeNode* down;
	TreeNode* up;
	list<TreeNode*> down;
	TreeNode* nextSimilar;
	TreeNode* prevSimilar;
};

//struct ConnectionElement {
//	int elementValue;
//	int elementFrequency;
//};

struct ConnectionRow {
	TreeNode* firstOccurrence;
	map <int, int> connectedElements; //key value is element number and the other value is frequency
	TreeNode* lastOccurrence;
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
	
	vector <ConnectionRow*> connectionTable;
	vector<int> dbElementFrequency; //Vector created to store the frequency of each element
	string sTransaction;  //One Transaction information in string
	list<int> iTransaction;  //List of integers for each transaction

	void ExtractIntegersToList();
	void AddElementFrequency();
	void AddToTree();
	void AddToConnectionTable();
	TreeNode* AllocateTreeNodeMemory(int value);
	ConnectionRow* AllocateConnectionRow();
	void clearPreviousWindow();
	void DeleteTreeNodes();

public:
	SRPTree();

	int Initialize();
	int Finalize();
	int ReadTransaction();
	int GetWindowSize();
	void Mine();

	//void GetFreqMinSup();
};
