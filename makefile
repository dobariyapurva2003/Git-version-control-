# Define the compiler and compiler flags
CXX = g++
CXXFLAGS = -Wall -g

# Define the target executable
TARGET = a.out

# Define the source files
SRCS = main.c++ io.c++ pinfo.c++ sysCmd.c++ pipe.c++ ls.c++ echo.c++ pwd.c++ cd.c++ search.c++ 

# Define the object files
OBJS = $(SRCS:.cpp=.o)

# Default target: build the executable
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

