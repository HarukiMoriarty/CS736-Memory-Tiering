CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++17 -pthread -DBOOST_LOG_DYN_LINK
LDFLAGS = -lboost_log_setup -lboost_log -lboost_filesystem -lboost_thread -lboost_system -lboost_chrono -lpthread

SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
SRC = $(wildcard $(SRC_DIR)/*.cpp) main.cpp
OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRC)))
TARGET = $(BUILD_DIR)/main

all: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
