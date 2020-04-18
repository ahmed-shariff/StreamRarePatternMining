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

	rootNode = new(nothrow) TreeNode();
	if (rootNode)
	{
		//rootNode->up= rootNode->down = rootNode->nextSimilar  = NULL;
		//rootNode->elementValue = 0;
		rootNode->elementFrequency = 1;
	}
	else
	{
		cout << "Memory allocation failure"<< endl;
		return 0;
	}

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
		//Build tree
		AddToTree();
		list<int>::iterator it;
		cout << endl;
		for (it = iTransaction.begin(); it != iTransaction.end(); ++it)
			std::cout << ' ' << *it;
		cout << endl;

		//Clear the list for the next transaction
		iTransaction.clear();

		return 0; //temporary
		//return 1;
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
	rootNode;

}

void SRPTree::Mine()
{

}
