# Лаба 2 собирается поверх лабы 1: структуры берутся из ../lab1 (стек, массив, списки, очередь, дерево).
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I../lab1/include -Iinclude
LAB1     = ../lab1
BUILD    = build

LAB1_OBJ = $(patsubst $(LAB1)/src/%.cpp,$(BUILD)/lab1_%.o,$(wildcard $(LAB1)/src/*.cpp))
LAB2_OBJ = $(BUILD)/main.o $(patsubst src/%.cpp,$(BUILD)/%.o,$(wildcard src/*.cpp))
HEADERS  = $(wildcard include/*.h) $(wildcard $(LAB1)/include/*.h)

dbms: $(LAB2_OBJ) $(LAB1_OBJ)
	$(CXX) $^ -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/lab1_%.o: $(LAB1)/src/%.cpp $(wildcard $(LAB1)/include/*.h) | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/main.o: main.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) lab2

.PHONY: clean
