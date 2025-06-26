# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread

# Target binary name
TARGET = SensorPublisher

# Source files
SRCS = SensorPublisher.cpp

# Build rule
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)



clean:
	rm -f $(TARGET)