# Makefile for ppw-server

CXX := aarch64-linux-gnu-g++
CXXFLAGS := -O2 -Wall -std=c++17

TARGET := ppw-server
SRC := main.cpp server.cpp
BUILD_DIR := build

OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean