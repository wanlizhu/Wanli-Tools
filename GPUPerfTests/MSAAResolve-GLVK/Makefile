CC = gcc
CXX = g++
CFLAGS = -O2 -Wall -Wextra
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LIBS = -lglfw -lGL -ldl

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
	sudo apt-get install -y build-essential libglfw3-dev libgl1-mesa-dev mesa-common-dev libglu1-mesa-dev

# Run the application (always uses NVIDIA - throws exception if not available)
run: $(TARGET)
	@echo "Running NVIDIA-only OpenGL MSAA test..."
	./$(TARGET)

# Check GPU information
gpu-info:
	@echo "=== System GPU Information ==="
	@echo "Available GPUs:"
	@lspci | grep -E "(VGA|3D)" || echo "No GPU information available"
	@echo
	@echo "OpenGL with Mesa (default):"
	@glxinfo | grep -E "(OpenGL vendor|OpenGL renderer|OpenGL version)" 2>/dev/null || echo "glxinfo not available (install mesa-utils)"
	@echo
	@echo "OpenGL with NVIDIA (forced by application):"
	@__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia glxinfo | grep -E "(OpenGL vendor|OpenGL renderer|OpenGL version)" 2>/dev/null || echo "NVIDIA driver not available or not working"
	@echo
	@echo "GLFW version:"
	@pkg-config --modversion glfw3 2>/dev/null || echo "GLFW not installed (install libglfw3-dev)"

.PHONY: all clean install-deps run gpu-info
