# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread
LDFLAGS = -lgpiod
# Target binary name
TARGETS = SensorPublisher LedService BuzzerService WatchdogManager


%: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

all: $(TARGETS)

clean:
	rm -f $(TARGETS)