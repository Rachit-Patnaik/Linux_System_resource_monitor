CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -Isrc
TARGET = res_monitor_cpp

SRCS = src/main.cpp src/ProcessMonitor.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean