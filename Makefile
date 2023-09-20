all:
	g++ main.cpp block.cpp libs/SHA256.cpp -o main && ./main
