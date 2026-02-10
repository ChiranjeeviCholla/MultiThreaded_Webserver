# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# Directories
SRC_DIR = src
INC_DIR = inc

# Source files
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/Server.cpp \
       $(SRC_DIR)/Threadpool.cpp

# Output binary
TARGET = server

# Build target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SRCS) -o $(TARGET)

# Clean build
clean:
	rm -f $(TARGET)

# Rebuild from scratch
rebuild: clean all

