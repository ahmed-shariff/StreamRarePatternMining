#include "SRPTree.h"
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
	useDfs = false;
	mineRareItemItemsets = false;
}

void SRPTree::ClearWhiteSpace()
{
	sConfigFileLine.erase(remove(sConfigFileLine.begin(), sConfigFileLine.end(), ' '), sConfigFileLine.end());
	sConfigFileLine.erase(remove(sConfigFileLine.begin(), sConfigFileLine.end(), '\t'), sConfigFileLine.end());
}
int SRPTree::Initialize()
{
	bool foundConfiguration = false;
	int activeConfiguration = -1;
	configStream.open("miningsettings.config");

	if (!configStream) {
		cout << "ConfigFile not present..." << endl;
		return 0;
	}
	
	while (getline(configStream, sConfigFileLine)) {
		if (activeConfiguration == -1)
		{
			if (sConfigFileLine.find("Active Configuration:") != string::npos)
			{
				//string s = sConfigFileLine.replace(sConfigFileLine.find("Active Configuration:"), 21, "").c_str();
				sConfigFileLine.replace(sConfigFileLine.find("Active Configuration:"), 21, "");
				ClearWhiteSpace();
				activeConfiguration = atoi(sConfigFileLine.c_str());
			}
			continue;
		}

		if (!foundConfiguration && sConfigFileLine.find("Configuration 		:") != string::npos)
		{
			if (sConfigFileLine.find("Configuration 		:") != string::npos)
			{
				sConfigFileLine.replace(sConfigFileLine.find("Configuration 		:"), 17, "");
				ClearWhiteSpace();

				if (atoi(sConfigFileLine.c_str()) == activeConfiguration)
				{
					foundConfiguration = true;
				}
			}
			continue;
		}
		if (foundConfiguration)
		{
			if (sConfigFileLine.find("DatasetFilename") != string::npos)
			{
				sConfigFileLine.replace(sConfigFileLine.find("DatasetFilename		: 	"), 18, "");
				ClearWhiteSpace();
				filename = sConfigFileLine.c_str();
			}
			else
			{
				continue;
			}

			getline(configStream, sConfigFileLine);
			sConfigFileLine.replace(sConfigFileLine.find("RareMinSup			:"), 14, "");
			ClearWhiteSpace();
			rareMinSup = atoi((sConfigFileLine.c_str()));

			getline(configStream, sConfigFileLine);
			sConfigFileLine.replace(sConfigFileLine.find("FreqMinSup			:"), 14, "");
			ClearWhiteSpace();
			freqMinSup = atoi(sConfigFileLine.c_str());
			
			getline(configStream, sConfigFileLine);
			sConfigFileLine.replace(sConfigFileLine.find("WindowSize			:"), 14, "");
			ClearWhiteSpace();
			windowSize = atoi(sConfigFileLine.c_str());

			break;
		}

		sConfigFileLine.clear();
	}
	if (!foundConfiguration)
	{
		cout << "Configuration Not found" << endl;
	}
	
	cout << "Database name is " << filename.data() << endl;
	
	//Open file
	in.open(filename);

	if (!in) {
		cout << "Database file not present. Please check the file." << endl;
		return 0;
	}

	cout << "rareMinSup " << rareMinSup << endl;
	cout << "freqMinSup " << freqMinSup << endl;
	cout << "windowSize " << windowSize << endl;

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
		_dfs(*it, searchItem, returnList);
}


void _get_transactions(TreeNode* currentNode, TreeNode* rootNode, vector<Transaction<int>> *conditionalBase){
	Transaction<int> _temp_transaction;
	int minSupport = numeric_limits<int>::max();
	while(currentNode != rootNode)
	{
		if(currentNode->elementFrequency < minSupport)
			minSupport = currentNode->elementFrequency;
		_temp_transaction.push_back(currentNode->elementValue);
		currentNode = currentNode->up;
	}
	
	for (int i=0; i < minSupport; i++) {
		// for (auto e: _temp_transaction)
		// 	cout << e << " ";
		// cout << endl;
		conditionalBase -> push_back(_temp_transaction);
	}
}

map<set<int>, int> SRPTree::_MineRareItemItemsets()
{
	cout << "mining" << endl;
	set<int> searchElements;
	unsigned int i;

	int f;
	// get rare items
	for(map<int, ConnectionRow*>::iterator it = connectionTable.begin(); it != connectionTable.end(); it ++)
	{
		f = (*it).second->elementFrequency;
		if( f >= rareMinSup && f < freqMinSup )
			searchElements.insert((*it).first);
	}
	
	set<int> rareItems(searchElements);
	map <int, int> _connectedElements;
	
	// get items co occuring with rare items
	//for (auto rareElement: rareItems)
	for (set<int>::iterator it = rareItems.begin(); it != rareItems.end(); it++)
	{
		map<int, int> _connectedElements = connectionTable[(*it)]->connectedElements;
		for(map<int, int>::iterator mapit = _connectedElements.begin(); mapit != _connectedElements.end(); mapit++){
			if ( (*mapit).second >= rareMinSup )
				searchElements.insert((*mapit).first);
		}
	}

	list<TreeNode*>::iterator listIt;
	vector<Transaction<int>> conditionalBase;
	TreeNode *currentNode;
	ConnectionRow currentRow; // when using horizontal connections
	list<TreeNode*> searchList; // when using the dfs
	map<set<int>, int> rarePatterns; // can we guarentee that the same itemsets will not be repeated?
	set<set<int>> blacklisted;
	/*
	 * 1. Build the conditional base for each item in R
	 * 2. Apply FP growth on it.
	 * TODO: The FP growth sill prune none rare itemsets, how?
	 */
	for (set<int>::iterator searchElement = searchElements.begin(); searchElement != searchElements.end(); searchElement++)
		//for (auto searchElement: searchElements)
	{
		conditionalBase.clear();
		// cout << "*****" << searchElement << endl;
		if(useDfs){
			searchList.clear();                                                
			_dfs(rootNode, *searchElement, &searchList);
			for (listIt=searchList.begin(); listIt!=searchList.end(); listIt++)
			{
				currentNode = *listIt;
				_get_transactions(currentNode, rootNode, &conditionalBase);
			}
		}else{
			currentRow = *connectionTable[*searchElement];
			// cout << searchElement << "-" << currentRow.elementFrequency << endl;
			currentNode = currentRow.firstOccurrence;
			while(currentNode)
			{
				//cout << "--"<< endl;
				_get_transactions(currentNode, rootNode, &conditionalBase);
				currentNode = currentNode->nextSimilar;
			}
		}
		FPTree<int> fptree(conditionalBase, rareMinSup);
		
		// making sure the header table in the FPtree only has the item we are looking at
		// shared_ptr<FPNode<int>> headerTableItem = fptree.header_table[searchElement];
		// fptree.header_table.clear();
		// fptree.header_table.insert(pair<int, shared_ptr<FPNode<int>>>(searchElement, headerTableItem));
		
		const std::set<Pattern<int>> patterns = fptree_growth( fptree );
		for (set<Pattern<int>>::iterator it = patterns.begin(); it != patterns.end(); it++)
			//for (auto [el, f]: patterns)
		{
			set<int> el = (*it).first;
			int f = (*it).second;
			if (f >= rareMinSup && f < freqMinSup){
				if (el.size() == 1){
					if (rareItems.find(*el.begin()) != rareItems.end())
						rarePatterns.insert(pair<set<int>, int>(el, f));
				}else{
					if (blacklisted.find(el) == blacklisted.end()){
						if (rarePatterns.find(el) == rarePatterns.end())
							rarePatterns.insert(pair<set<int>, int>(el, f));
						else if (rarePatterns[el] < f)
							rarePatterns[el] = f;
					}
				}
			}else{
				if (rarePatterns.find(el) != rarePatterns.end()){
					rarePatterns.erase(el);
				}
				blacklisted.insert(el);
			}
		}
			
	}

	cout << "Patterns detected: " << endl;
	for (auto p: rarePatterns) {
		cout << "{ ";
		for (auto s: p.first){
			cout << s << " ";
		}
		cout << "} \t\tf: "<< p.second << endl;
	}
	clearPreviousWindow();
	return rarePatterns;
}

int _dfs_mine_transactions(TreeNode node, vector<Transaction<int>> *conditionalBase, Transaction<int> transaction, int minValue){
	// if(node.elementFrequency < minValue){
	// 	if(transaction.size() > 0){
	// 		for(int i = 0 ; i < minValue; i++){
	// 			Transaction<int> _transaction(transaction);
	// 			conditionalBase->push_back(transaction);
	// 		}
	// 	}
	// 	minValue = node.elementFrequency;
	// }
	// if(node.down.size() == 0){
	// 	transaction.push_back(node.elementValue);
	// 	for(int i = 0 ; i < minValue; i++){
	// 		conditionalBase->push_back(transaction);
	// 	}
	// }else{
	if (node.up)
		transaction.push_back(node.elementValue);
	int childMinValueSum = 0;
	for (list<TreeNode*>::iterator it = node.down.begin(); it != node.down.end(); it++ ){
		childMinValueSum += _dfs_mine_transactions(*(*it), conditionalBase, transaction, minValue);
	}
	if ( node.up && node.elementFrequency > childMinValueSum ){
		for (int i = 0; i < (node.elementFrequency - childMinValueSum); i++) {
			Transaction<int> _transaction(transaction);
			conditionalBase->push_back(_transaction);
		}
	}
	return node.elementFrequency;
}

map<set<int>, int> SRPTree::_MineNonRareItemItemsets()
{
	map<set<int>, int> rarePatterns;
	vector<Transaction<int>> conditionalBase;

	_dfs_mine_transactions(*rootNode, &conditionalBase, *(new Transaction<int>()), numeric_limits<int>::max());
	// for (auto t: conditionalBase){
	// 	for (auto e: t)
	// 		cout << e << " " ;
	// 	cout << endl;
	// }
	FPTree<int> fptree(conditionalBase, rareMinSup);
		
	// making sure the header table in the FPtree only has the item we are looking at
	// shared_ptr<FPNode<int>> headerTableItem = fptree.header_table[searchElement];
	// fptree.header_table.clear();
	// fptree.header_table.insert(pair<int, shared_ptr<FPNode<int>>>(searchElement, headerTableItem));
		
	const std::set<Pattern<int>> patterns = fptree_growth( fptree );
	for (set<Pattern<int>>::iterator it = patterns.begin(); it != patterns.end(); it++)
		//for (auto [el, f]: patterns)
	{
		set<int> el = (*it).first;
		int f = (*it).second;
		if (f >= rareMinSup && f < freqMinSup){
			rarePatterns.insert(pair<set<int>, int>(el, f));
		}
		// if (f >= rareMinSup && f < freqMinSup){
		// 	if (el.size() == 1){
		// 		if (rareItems.find(*el.begin()) != rareItems.end())
		// 			rarePatterns.insert(pair<set<int>, int>(el, f));
		// 	}else{
		// 		if (blacklisted.find(el) == blacklisted.end()){
		// 			if (rarePatterns.find(el) == rarePatterns.end())
		// 				rarePatterns.insert(pair<set<int>, int>(el, f));
		// 			else if (rarePatterns[el] < f)
		// 				rarePatterns[el] = f;
		// 		}
		// 	}
		// }else{
		// 	if (rarePatterns.find(el) != rarePatterns.end()){
		// 		rarePatterns.erase(el);
		// 	}
		// 	blacklisted.insert(el);
		// }
	}
	
	
	cout << "Patterns detected: " << endl;
	for (auto p: rarePatterns) {
		cout << "{ ";
		for (auto s: p.first){
			cout << s << " ";
		}
		cout << "} \t\tf: "<< p.second << endl;
	}
	clearPreviousWindow();
	return rarePatterns;
}

map<set<int>, int> SRPTree::Mine()
{
	cout << "mining" << endl;
	if (mineRareItemItemsets)
		return _MineRareItemItemsets();
	else
		return _MineNonRareItemItemsets();
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
