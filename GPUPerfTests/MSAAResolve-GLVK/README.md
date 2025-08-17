# OpenGL MSAA Resolve Test

This is an OpenGL implementation of the MSAA (Multi-Sample Anti-Aliasing) resolve test, equivalent to the DirectX 12 version in MSAAResolve-DX12.

## What it does

The test renders a fullscreen triangle that generates random black/white pixels based on pixel coordinates and frame count, using the highest supported MSAA level (automatically detected from 16x down to 1x). It then performs an MSAA resolve operation to combine the multiple samples into a single sample per pixel and displays the result.

## Features

- **NVIDIA ONLY**: **REQUIRES** NVIDIA OpenGL hardware acceleration - throws exceptions if not available
- **GLFW Window Management**: Uses GLFW3 for reliable cross-platform window and context creation
- **Adaptive MSAA**: Automatically finds best supported MSAA level on NVIDIA hardware (16x → 8x → 4x → 2x → 0x)
- **4K Resolution**: Renders at 3840x2160 resolution
- **MSAA Resolve**: Demonstrates OpenGL's MSAA resolve using glBlitFramebuffer
- **Random Pattern**: Generates animated random black/white pixel pattern
- **GLAD Integration**: Uses GLAD for dynamic OpenGL function loading
- **Strict Hardware Validation**: Multiple checks ensure NVIDIA hardware rendering (no Mesa fallback)
- **Comprehensive Error Reporting**: Detailed diagnostics for NVIDIA driver issues
- **Robust Context Creation**: GLFW handles complex OpenGL context setup automatically

## Requirements

### Hardware & Drivers (MANDATORY)
- **NVIDIA GPU**: GeForce, Quadro, or Tesla series
- **NVIDIA proprietary drivers**: Version 470+ recommended
- **PRIME configuration**: Must be set to NVIDIA (if hybrid graphics)

### System Requirements  
- Linux system with graphics display (X11 or Wayland via XWayland)
- OpenGL 4.5+ support via NVIDIA drivers
- Development packages: build-essential, libglfw3-dev, libgl1-mesa-dev, mesa-common-dev, libglu1-mesa-dev
- C/C++ compiler (gcc/g++)

## Building

### Ubuntu/Debian
```bash
# Install dependencies
make install-deps

# Build the project
make
```

### Manual build (if needed)
```bash
# Create output directory and compile sources separately, then link
mkdir -p _out
gcc -O2 -Wall -Wextra -c glad.c -o _out/glad.o
g++ -std=c++17 -O2 -Wall -Wextra -c main.cpp -o _out/main.o
g++ -std=c++17 -O2 -Wall -Wextra -o _out/MSAAResolve-GLVK _out/main.o _out/glad.o -lglfw -lGL -ldl
```

## Running

### NVIDIA ONLY MODE
**This application REQUIRES NVIDIA OpenGL hardware acceleration.**
**It will THROW EXCEPTIONS and refuse to run with Mesa/llvmpipe software rendering.**

```bash
# Run the application (NVIDIA required)
./_out/MSAAResolve-GLVK

# Or use the Makefile target
make run
```

### Prerequisites
Before running, ensure NVIDIA drivers are properly installed:
```bash
# Check if NVIDIA GPU is detected
nvidia-smi

# Check GPU information 
make gpu-info

# Install NVIDIA drivers if needed (Ubuntu/Debian)
sudo apt install nvidia-driver-470  # or latest version
sudo reboot
```

### Controls
- **ESC**: Exit the application
- **Close button**: Exit the application

## Technical Details

### Window Management & Context Creation
- Uses GLFW3 for cross-platform window and OpenGL context management
- Automatic handling of complex GLX/WGL context creation details
- Built-in support for MSAA framebuffer configuration
- Robust error handling for context creation failures

### OpenGL Function Loading
- Uses GLAD (glad.c/glad.h) for dynamic loading of OpenGL functions
- Supports OpenGL 4.5 compatibility profile with extensions
- Initializes with GLFW's `glfwGetProcAddress` for proper function loading

### Enhanced MSAA Detection & Configuration
- GLFW-based MSAA testing: automatically tries 16x → 8x → 4x → 2x → 0x samples
- Uses temporary windows to test actual MSAA support before main window creation
- Real-time verification of achieved sample counts via `glGetIntegerv(GL_SAMPLES)`
- Graceful fallback to lower MSAA levels with clear reporting

### Shaders
- `vertex-shader.glsl`: Generates fullscreen triangle using gl_VertexID
- `fragment-shader.glsl`: Creates random black/white pattern based on pixel coordinates and frame count

### MSAA Process
1. Render to MSAA framebuffer (8 samples per pixel)
2. Resolve MSAA framebuffer to regular framebuffer using `glBlitFramebuffer`
3. Copy resolved image to back buffer for display

### Comparison with DX12 Version
This OpenGL implementation maintains the same functionality as the DirectX 12 version:
- Same resolution (4K)
- Same MSAA sample count (8x)
- Same random pattern generation algorithm
- Same frame-based animation

The main differences are in the API usage:
- X11 instead of Win32 for windowing
- OpenGL instead of DirectX 12 for graphics
- GLX instead of DXGI for context creation  
- `glBlitFramebuffer` instead of `ResolveSubresource` for MSAA resolve
- GLAD for dynamic OpenGL function loading

## Project Structure

```
MSAAResolve-GLVK/
├── main.cpp              # Main application with GLFW/OpenGL code
├── glad.c/.h             # OpenGL function loader (included)
├── vertex-shader.glsl    # GLSL vertex shader
├── fragment-shader.glsl  # GLSL fragment shader  
├── Makefile             # Build configuration
├── README.md            # This file
└── _out/                # Build output directory (created by make)
    ├── main.o           # Compiled C++ object file
    ├── glad.o           # Compiled C object file  
    └── MSAAResolve-GLVK # Final executable
```

## Build Output

After successful compilation, the `_out/` directory will contain:
- `_out/MSAAResolve-GLVK`: Main executable
- `_out/main.o`, `_out/glad.o`: Object files

All build artifacts are contained in the `_out/` directory, which can be completely removed with `make clean`.

The application creates a 4K window and renders animated random patterns using the best available MSAA level, demonstrating OpenGL's multisampling capabilities equivalent to the DirectX 12 version.

## Example Output

When running successfully, you'll see output like:
```
OpenGL MSAA Resolve Test - Starting...
GLX version: 1.4
Searching for best MSAA configuration...
  Trying 16x MSAA...
  No configurations found for 16x MSAA
  Trying 8x MSAA...
  Found configuration with 8 samples (12 configurations available)
Selected MSAA configuration: 8x samples
Successfully configured 8x MSAA
OpenGL Vendor: NVIDIA Corporation
OpenGL Renderer: NVIDIA GeForce RTX 3080
OpenGL Version: 4.6.0 NVIDIA 470.86
```

The window title will show the actual MSAA level being used: `"OpenGL MSAA Resolve Test - 4K (8x MSAA)"`

## Troubleshooting

### Library Linking Errors
If you get "cannot find X11/GL/dl" errors:
```bash
# Make sure development packages are installed
make install-deps

# On some distributions, you might need additional packages:
sudo apt-get install libgl1-mesa-glx libx11-6
```

### Build Errors  
```bash
# Clean and rebuild
make clean
make

# Check if all required files are present
ls -la main.cpp glad.c glad.h vertex-shader.glsl fragment-shader.glsl

# Check build output
ls -la _out/
```

### Runtime Issues
- Graphics drivers must support OpenGL 4.5+
- For Wayland users: ensure XWayland is available (GLFW will use it automatically)
- For remote connections: X11 forwarding or native Wayland support required

### Application Crashes with NVIDIA Errors
**This application will THROW EXCEPTIONS and terminate if NVIDIA drivers are not working properly.**

**Common error messages and solutions:**

#### "NVIDIA driver may not be properly installed"
```bash
# Install NVIDIA drivers
sudo apt install nvidia-driver-470  # or latest version
sudo reboot

# Verify installation
nvidia-smi
lspci | grep -i nvidia
```

#### "Got indirect rendering instead of NVIDIA hardware acceleration"
```bash
# Check PRIME configuration (hybrid graphics)
prime-select query
sudo prime-select nvidia
sudo reboot

# Add user to video group
sudo usermod -a -G video $USER
sudo reboot
```

#### "OpenGL vendor is NOT NVIDIA"
```bash
# Check what OpenGL vendor is being used
glxinfo | grep -i "opengl vendor"

# Force NVIDIA environment (shouldn't be needed with new code)
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia

# Check GPU detection
make gpu-info
```

#### "Failed to create NVIDIA OpenGL 4.5 context"  
- Update NVIDIA drivers to newer version
- Check if GPU supports OpenGL 4.5: `glxinfo | grep "OpenGL version"`
- Disable secure boot if using proprietary drivers
- Check BIOS settings - ensure discrete GPU is enabled

**The application WILL NOT run with Mesa/software rendering - this is intentional!**
