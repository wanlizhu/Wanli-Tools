CC = gcc
CXX = g++
CFLAGS = -O2 -Wall -Wextra
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LIBS = -lX11 -lGL -ldl

TARGET = MSAAResolve-GLVK
CPP_SOURCES = main.cpp
C_SOURCES = glad.c
OBJECTS = $(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential libx11-dev libgl1-mesa-dev mesa-common-dev libglu1-mesa-dev

.PHONY: all clean install-deps install-deps-rpm check-libs build-pkgconfig
