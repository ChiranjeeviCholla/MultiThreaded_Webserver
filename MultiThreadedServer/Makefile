CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread

SRC = src/main.cpp src/Server.cpp src/ThreadPool.cpp
INC = -Iinclude

TARGET = server

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(INC) -o $(TARGET)

clean:
	rm -f server
