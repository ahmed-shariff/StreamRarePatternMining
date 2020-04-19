#include "SRPTree.h"

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

	connectionTable.resize(distinctElements);
	dbElementFrequency.resize(distinctElements);

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
	cout << "dbElementFrequency.size " << dbElementFrequency.size() << endl;

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
		//Read the transaction as a string
		getline(in, sTransaction);  // delim defaults to '\n'
		//Store it as integer in a list
		ExtractIntegersToList();
		//Sort the list in canonical order
		iTransaction.sort();
		//Update element frequency 
		AddElementFrequency();
		//Add it to the connection table
		AddToConnectionTable();
		//Build tree
		AddToTree();
		list<int>::iterator it;
		cout << endl;
		for (it = iTransaction.begin(); it != iTransaction.end(); ++it)
			std::cout << ' ' << *it;
		cout << endl;

		//Clear the list for the next transaction
		iTransaction.clear();

		//return 0; //temporary
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
	
	for (it = iTransaction.begin(); it != iTransaction.end(); ++it)
	{
		cout<<"before" <<dbElementFrequency[*it]++ << "after"<<dbElementFrequency[*it] <<endl;
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
				(*it)->elementValue++;
				found = true;
				iTransaction.pop_front();
				currentNode = *it;
			}
		}
		
		if (!found)
		{
			newNode = AllocateTreeNodeMemory(iTransaction.front());
			currentNode->down.push_front(newNode);
			iTransaction.pop_front();
			newNode->up = currentNode;
			currentNode = newNode;
			
			if (!connectionTable[iTransaction.front()]->firstOccurrence)
			{
				connectionTable[iTransaction.front()]->firstOccurrence = newNode;
				connectionTable[iTransaction.front()]->lastOccurrence = newNode;
			}
			else
			{
				newNode->prevSimilar = connectionTable[iTransaction.front()]->lastOccurrence;
				connectionTable[iTransaction.front()]->lastOccurrence->nextSimilar = newNode;
				connectionTable[iTransaction.front()]->lastOccurrence = newNode;
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
		if (!connectionTable[*it])
		{
			connectionTable[*it] = AllocateConnectionRow();	
		}
		std::list<int>::iterator itmid = it;
		map<int, int>::iterator mapIterator;
		itmid++;

		for (itmid; itmid != iTransaction.end(); ++itmid)
		{
			mapIterator = connectionTable[*it]->connectedElements.find(*(itmid));
			if(mapIterator == connectionTable[*itmid]->connectedElements.end())
			{ 
				connectionTable[*it]->connectedElements.insert(pair<int, int>(*itmid, 1));
			}
			else
			{
				mapIterator->second++;
			}
		}
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

void SRPTree::Mine()
{

}

void SRPTree::clearPreviousWindow()
{

	//DeleteTreeNodes();
	//DeleteAllocateConnectionRow();

	//delete vector dbElementFrequency;
	//delete connectionTable; //delete all vectors
	//delete connectedElements maps; //connection row deletion shoudl clear this memory

	//connectionTable[*it]->connectedElements.erase(connectionTable[*it]->connectedElements.begin(), connectionTable[*it]->connectedElements.end());
}

void SRPTree::DeleteTreeNodes()
{

}
