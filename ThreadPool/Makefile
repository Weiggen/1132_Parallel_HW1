CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O3 -pthread
TARGET = thread_pool
SRC = hw1_threadPool_312611101.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean