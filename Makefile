all:
	g++ main.cpp block.cpp -o main -lcrypto -static-libstdc++ && ./main
