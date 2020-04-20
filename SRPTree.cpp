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

	//connectionTable.resize(distinctElements);
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

		return 0; //temporary
	//	return 1;
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

		if (connectionTableIterator != connectionTable.end())
		{
			connectionTableIterator->second->elementFrequency++;
		}
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

void SRPTree::Mine()
{
	clearPreviousWindow();
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
}

void SRPTree::DeleteTreeNodes()
{
	//Without tree traversal
	for (std::map<int, ConnectionRow*>::reverse_iterator  connectionTableIterator = connectionTable.rbegin(); connectionTableIterator != connectionTable.rend(); ++connectionTableIterator)
	{
		while (connectionTableIterator->second->lastOccurrence->prevSimilar)
		{
			if (connectionTableIterator->second->lastOccurrence->up == rootNode)
			{
				for (std::list<TreeNode*>::iterator it = rootNode->down.begin(); it != rootNode->down.end(); ++it)
					if(*it == connectionTableIterator->second->lastOccurrence)
					{
						rootNode->down.erase(it);
					}
			}
			TreeNode* previousOccurence = connectionTableIterator->second->lastOccurrence->prevSimilar;
			delete connectionTableIterator->second->lastOccurrence;
			connectionTableIterator->second->lastOccurrence = previousOccurence;
		}
		connectionTableIterator->second->firstOccurrence = NULL;
		connectionTableIterator->second->lastOccurrence = NULL;
	}

}
