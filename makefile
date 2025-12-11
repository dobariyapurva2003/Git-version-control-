# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Libraries to link
LIBS = -lssl -lcrypto -lz

# Target executable name
TARGET = mygit

# Source file
SRC = main.cpp

# Rule to build the target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean up the compiled files
clean:
	rm -f $(TARGET)
