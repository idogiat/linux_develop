# Compiler and flags
CXX = g++
DEBUG_FLAGS = -g
CXXFLAGS = -std=c++17 -Wall -pthread $(DEBUG_FLAGS)
LDFLAGS = -lgpiod

# Target binary name
TARGETS = SensorPublisher LedService BuzzerService
OBJ = SensorPublisher.o LedService.o BuzzerService.o
# COMMON_OBJ =

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(LDFLAGS)

%: %.o
	$(CXX) $(CXXFLAGS) $(COMMON_OBJ) $^ -o $@ $(LDFLAGS)


all: $(COMMON_OBJ) $(OBJ) $(TARGETS) 

clean:
	rm -f $(COMMON_OBJ) $(OBJ) $(TARGETS) 
