CXX    = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g

TARGET = buffer_mgr
SRCS   = buffer_core.cpp evict.cpp

all: $(TARGET)

$(TARGET): $(SRCS) buffer.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
