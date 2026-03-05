CXX := g++
CXXFLAGS := -O2 -Wall -std=c++17

TARGET := ppw-server
SRC := main.cpp server.cpp

BUILD_DIR := build
BIN_DIR := bin

OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)
BIN_TARGET := $(BIN_DIR)/$(TARGET)

all: $(BIN_TARGET)

# Link executable into BIN_DIR
$(BIN_TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into BUILD_DIR
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean