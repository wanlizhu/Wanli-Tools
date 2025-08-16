CC = gcc
CXX = g++
CFLAGS = -O2 -Wall -Wextra
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LIBS = -lX11 -lGL -ldl

OUT_DIR = _out
TARGET = $(OUT_DIR)/MSAAResolve-GLVK
CPP_SOURCES = main.cpp
C_SOURCES = glad.c
OBJECTS = $(CPP_SOURCES:%.cpp=$(OUT_DIR)/%.o) $(C_SOURCES:%.c=$(OUT_DIR)/%.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS) | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(OUT_DIR)/%.o: %.cpp | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT_DIR)/%.o: %.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -rf $(OUT_DIR)

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential libx11-dev libgl1-mesa-dev mesa-common-dev libglu1-mesa-dev

.PHONY: all clean install-deps
