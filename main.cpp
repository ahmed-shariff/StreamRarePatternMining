#include <iostream>
#include "SRPTree.h"

using namespace std;

int main()
{
	SRPTree srpTree;
	if (!srpTree.Initialize())
	{
		cout << "Initialization failed. Exiting...<<endl";
		return 0;
	}

	int readCount = 0;

	while (srpTree.ReadTransaction())
	{
		readCount++;
		if (readCount == srpTree.GetWindowSize())
		{
			srpTree.Mine();
			readCount = 0;
		}
		
	}

	srpTree.Finalize();

	system("PAUSE");
	return 0;
}
