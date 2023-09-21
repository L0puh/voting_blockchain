CXX=g++
LDFLAGS=-lcrypto -static-libstdc++ -Iinclude
SRC=src/main.cpp src/block.cpp
EXECUTABLE=voting
.PHONY: all clean

all: 
	$(CXX) $(SRC) -o $(EXECUTABLE) $(LDFLAGS) 

clean:
	rm -f $(EXECUTABLE)
