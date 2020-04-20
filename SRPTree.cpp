#include "SRPTree.h"
#include <iostream>
#include <vector>
#include <set>
#include "external/FP-growth/include/fptree.hpp"

SRPTree::SRPTree()
{
	rareMinSup = 0;
	freqMinSup = 0;
	windowSize = 1000; //number of transactions after which mining should start
	inputWindowSize = 0; //input window size given by a user
	distinctElements = 1024;
	inputDistinctElements = 0;
	//Change name of database file here
	filename = "T10I4D100K.dat.txt";
	useDfs = true;
}

int SRPTree::Initialize()
{
	cout << "Database name is " << filename.data() << endl;

	//Open file
	in.open(filename);

	if (!in) {
		cout << "Database file not present. Please check the file."<<endl;
		return 0;
	}

	//Small optimization for memory and perf 
	cout << "Enter the number of distinct elements for memory and perf optimization. Else, use '0' for default of 1024) "<<endl;
	cin >> inputDistinctElements;

	//The idea is based on the premise that the number of distinct elements will be continuous numbers
	if (inputDistinctElements != 0)
		distinctElements = inputDistinctElements;
	else
	{
		cout << "Using default value..."<<endl;
	}

	//connectionTable.resize(distinctElements);
	//dbElementFrequency.resize(distinctElements);

	cout << "Enter rareMinSup ";
	cin >> rareMinSup;
	cout << "Enter freqMinSup ";
	cin >> freqMinSup;
	cout << "Enter Windowsize (press 0 for default value of 1000) ";
	cin >> inputWindowSize;  
	if (inputWindowSize != 0)
		windowSize = inputWindowSize;
	   	  	
	cout << "rareMinSup" << rareMinSup<<endl;
	cout << "freqMinSup" << freqMinSup<<endl;
	//cout << "dbElementFrequency.size " << dbElementFrequency.size() << endl;

	//Allocate Root Node
	rootNode = AllocateTreeNodeMemory(0);
	
	return 1;
}

int SRPTree::Finalize()
{
	in.close();
	return 1;
}

int SRPTree::ReadTransaction()
{
	int count = 0;

	if (in) 
	{
		cout << endl;
		//Read the transaction as a string
		getline(in, sTransaction);  // delim defaults to '\n'
		//Store it as integer in a list
		ExtractIntegersToList();
		//Sort the list in canonical order
		iTransaction.sort();
		//Add it to the connection table
		AddToConnectionTable();
		//Update element frequency 
		AddElementFrequency();
		//Build tree
		AddToTree();
		list<int>::iterator it;
		cout << endl;
		//for (it = iTransaction.begin(); it != iTransaction.end(); ++it)
		//	std::cout << ' ' << *it;
		/*cout << endl;*/

		//Clear the list for the next transaction
		iTransaction.clear();

	//	return 0; //temporary
		return 1;
	}
	else
	{
		return 0;
	}
}

void SRPTree::ExtractIntegersToList()
{
	stringstream ss;

	/* Storing the whole string into string stream */
	ss << sTransaction;

	/* Running loop till the end of the stream */
	string temp;
	int found;
	while (!ss.eof()) {

		/* extracting word by word from stream */
		ss >> temp;

		/* Checking the given word is integer or not */
		if (stringstream(temp) >> found)
		{
			cout << found << " ";
			iTransaction.push_back(found);
		}

		/* To save from space at the end of string */
		temp = "";
	}
	sTransaction.clear();
}

void SRPTree::AddElementFrequency()
{
	list<int>::iterator it;
	map<int, ConnectionRow*>::iterator connectionTableIterator;
	
	for (it = iTransaction.begin(); it != iTransaction.end(); ++it)
	{
		//cout<<"before" <<dbElementFrequency[*it]++ << "after"<<dbElementFrequency[*it] <<endl;
	
		connectionTableIterator = connectionTable.find(*it);

	//	if (connectionTableIterator != connectionTable.end())
	//	{
			connectionTableIterator->second->elementFrequency++;
	//	}
		connectionTableIterator = connectionTable.end();
	}
}

int SRPTree::GetWindowSize()
{
	return windowSize;
}

void SRPTree::AddToTree()
{
	TreeNode* currentNode = rootNode;
	TreeNode* newNode = NULL;
	bool found = false;

	while (!iTransaction.empty())
	{
		found = false;
		list<TreeNode*>::iterator it;
		for (it = currentNode->down.begin(); it != currentNode->down.end(); ++it)
		{
			if ((*it)->elementValue == iTransaction.front())
			{
				(*it)->elementFrequency++;
				found = true;
				iTransaction.pop_front();
				currentNode = *it;
				break;
			}
		}
		
		if (!found)
		{
			newNode = AllocateTreeNodeMemory(iTransaction.front());
			currentNode->down.push_front(newNode);
			newNode->up = currentNode;
			currentNode = newNode;

			ConnectionRow* row = connectionTable[iTransaction.front()];
			iTransaction.pop_front();
			
			if (!row->firstOccurrence)
			{
				row->firstOccurrence = newNode;
				row->lastOccurrence = newNode;
			}
			else
			{
				newNode->prevSimilar = row->lastOccurrence;
				row->lastOccurrence->nextSimilar = newNode;
				row->lastOccurrence = newNode;
			}
		}
	}
}

TreeNode* SRPTree::AllocateTreeNodeMemory(int value)
{
	TreeNode* newNode = new(nothrow) TreeNode();
	if (newNode)
	{
		//newNode->up= newNode->down = newNode->nextSimilar  = NULL;
		newNode->elementValue = value;
		newNode->elementFrequency = 1;
		return newNode;
	}
	else
	{
		cout << "Memory allocation failure" << endl;
		return NULL;
	}
}

void SRPTree::AddToConnectionTable()
{
	for (std::list<int>::iterator it = iTransaction.begin(); it != iTransaction.end(); ++it)
	{
		map<int, ConnectionRow*>::iterator connectionTableIterator;

		connectionTableIterator = connectionTable.find(*it);

		if (connectionTableIterator == connectionTable.end())
		{
			connectionTable[*it] = AllocateConnectionRow();
			connectionTable[*it]->elementFrequency = 0;
		}
		std::list<int>::iterator itmid = it;
		map<int, int>::iterator mapIterator;
		itmid++;

		for (itmid; itmid != iTransaction.end(); ++itmid)
		{
			mapIterator = connectionTable[*it]->connectedElements.find(*(itmid));
			if(mapIterator == connectionTable[*it]->connectedElements.end())
			{ 
				connectionTable[*it]->connectedElements.insert(pair<int, int>(*itmid, 1));
			}
			else
			{
				mapIterator->second++;
			}
		}

		//if (!connectionTable[*it])
		//{
		//	connectionTable[*it] = AllocateConnectionRow();	
		//}
		//std::list<int>::iterator itmid = it;
		//map<int, int>::iterator mapIterator;
		//itmid++;

		//for (itmid; itmid != iTransaction.end(); ++itmid)
		//{
		//	mapIterator = connectionTable[*it]->connectedElements.find(*(itmid));
		//	if(mapIterator == connectionTable[*itmid]->connectedElements.end())
		//	{ 
		//		connectionTable[*it]->connectedElements.insert(pair<int, int>(*itmid, 1));
		//	}
		//	else
		//	{
		//		mapIterator->second++;
		//	}
		//}
	}
}

ConnectionRow* SRPTree::AllocateConnectionRow()
{
	ConnectionRow* newRow = new(nothrow) ConnectionRow();
	if (newRow)
	{
		return newRow;
	}
	else
	{
		cout << "Memory allocation failure" << endl;
		return NULL;
	}
}

void _dfs(TreeNode* node, int searchItem, list<TreeNode*> *returnList)
{
	if(node->elementValue == searchItem)
		(*returnList).push_back(node);
	list<TreeNode*>::iterator it;
	for (it = node->down.begin(); it != node->down.end(); ++it)
		_dfs(node, searchItem, returnList);
}

set<Pattern<int>> SRPTree::Mine()
{
	cout << "mining" << endl;
	clearPreviousWindow();
	set<int> searchElements;
	int i;

	int f;
	// get rare items
	for (i=0; i < connectionTable.size(); i++)
	{
		f = connectionTable[i]->elementFrequency;
		if( f >= rareMinSup && f < freqMinSup )
			searchElements.insert(i);
	}

	set<int> rareItems(searchElements);
	set<int>::iterator setIt;
	map <int, int> _connectedElements;
	// get items co occuring with rare items
	for (i = 0; i < connectionTable.size(); i++)
	{
		if( connectionTable[i]->elementFrequency >= freqMinSup){
			_connectedElements = connectionTable[i]->connectedElements;
			// Loop over the rare items and check if any of the rare items co occur with this item
			for (setIt = rareItems.begin(); setIt != rareItems.end(); setIt++) {
				if (_connectedElements.find(*setIt) != _connectedElements.end())
					searchElements.insert(i);
			}
		}
	}
	list<TreeNode*>::iterator listIt;
	vector<Transaction<int>> conditionalBase;
	TreeNode *currentNode;
	ConnectionRow currentRow; // when using horizontal connections
	list<TreeNode*> searchList; // when using the dfs
	set<Pattern<int>> rarePatterns; // can we guarentee that the same itemsets will not be repeated?
	/*
	 * 1. Build the conditional base for each item in R
	 * 2. Apply FP growth on it.
	 * TODO: The FP growth sill prune none rare itemsets, how?
	 */
	for (setIt = searchElements.begin(); setIt != searchElements.end();setIt++)
	{
		if(useDfs){
			searchList.clear();                                                
			_dfs(rootNode, *setIt, &searchList);                               
			for (listIt=searchList.begin(); listIt!=searchList.end(); listIt++)
			{
				currentNode = *listIt;
				Transaction<int> _temp_transaction;
				while(currentNode != rootNode)
				{
					_temp_transaction.push_back(currentNode->elementValue);
					currentNode = currentNode->up;
				}
				conditionalBase.push_back(_temp_transaction);	
			}
		}else{
		
			currentRow = *connectionTable[*setIt];
			currentNode = currentRow.firstOccurrence;
			while(currentNode != currentRow.lastOccurrence)
			{
				Transaction<int> _temp_transaction;
				while(currentNode != rootNode)
				{
					_temp_transaction.push_back(currentNode->elementValue);
					currentNode = currentNode->up;
				}
				conditionalBase.push_back(_temp_transaction);
				currentNode = currentNode->nextSimilar;
			}
		}
		FPTree<int> fptree(conditionalBase, rareMinSup);
		
		const std::set<Pattern<int>> patterns = fptree_growth( fptree );
		set<Pattern<int>>::iterator it;
		for (it=patterns.begin(); it != patterns.end(); it++)
		{
			rarePatterns.insert((*it)); // need to check if duplicates are being generated
		}
		
	}
	return rarePatterns;
}

void SRPTree::clearPreviousWindow()
{

	DeleteTreeNodes();

	map<int, ConnectionRow*>::iterator connectionTableIterator;

	for (std::map<int, ConnectionRow*>::iterator connectionTableIterator = connectionTable.begin(); connectionTableIterator != connectionTable.end(); ++connectionTableIterator)
	{
		connectionTableIterator->second->connectedElements.clear();
		delete connectionTableIterator->second;
	}
	connectionTable.clear();
}

void SRPTree::DeleteTreeNodes()
{
	cout << "deletion" << endl;
	//Without tree traversal
	for (std::map<int, ConnectionRow*>::reverse_iterator  connectionTableIterator = connectionTable.rbegin(); connectionTableIterator != connectionTable.rend(); ++connectionTableIterator)
	{
		cout << connectionTableIterator->first << " ";


		while (connectionTableIterator->second->lastOccurrence && connectionTableIterator->second->lastOccurrence->prevSimilar)
		{
			if (connectionTableIterator->second->lastOccurrence->up == rootNode)
			{
				for (std::list<TreeNode*>::iterator it = rootNode->down.begin(); it != rootNode->down.end(); ++it)
					if(*it == connectionTableIterator->second->lastOccurrence)
					{
						rootNode->down.erase(it);
						break;
					}
			}
			TreeNode* previousOccurence = connectionTableIterator->second->lastOccurrence->prevSimilar;
			delete connectionTableIterator->second->lastOccurrence;
			connectionTableIterator->second->lastOccurrence = previousOccurence;
		}

		if (!connectionTableIterator->second->lastOccurrence->prevSimilar)
		{
			if (connectionTableIterator->second->lastOccurrence->up == rootNode)
			{
				for (std::list<TreeNode*>::iterator it = rootNode->down.begin(); it != rootNode->down.end(); ++it)
					if (*it == connectionTableIterator->second->lastOccurrence)
					{
						rootNode->down.erase(it);
						break;
					}
			}
			TreeNode* previousOccurence = connectionTableIterator->second->lastOccurrence->prevSimilar;
			delete connectionTableIterator->second->lastOccurrence;
			connectionTableIterator->second->lastOccurrence = previousOccurence;
		}
		
		connectionTableIterator->second->firstOccurrence = NULL;
		connectionTableIterator->second->lastOccurrence = NULL;

		
	}
	cout << endl<< "elements left after deletion" << rootNode->down.size()<<endl;
}
