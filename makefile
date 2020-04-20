main:
	clang++ -Wall -std=c++11 SRPTree.cpp main.cpp -o build/main && build/./main

run:
	build/./main
