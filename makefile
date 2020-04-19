main:
	clang++ -Wall -std=c++11 SRPTree.cpp main.cpp -o build/out && build/./main

run:
	build/./main
