CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3 -pthread

TARGET = matrix_program
SOURCE = hw1_matrix_312611101.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
