CXX=g++
LDFLAGS=-lcrypto -static-libstdc++ -Iinclude -lpthread
SRC=src/main.cpp src/block.cpp src/network.cpp
EXECUTABLE=voting
.PHONY: all clean

all: 
	$(CXX) $(SRC) -o $(EXECUTABLE) $(LDFLAGS) 

clean:
	rm -f $(EXECUTABLE)
