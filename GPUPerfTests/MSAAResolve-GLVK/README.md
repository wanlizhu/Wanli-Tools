# OpenGL MSAA Resolve Test

This is an OpenGL implementation of the MSAA (Multi-Sample Anti-Aliasing) resolve test, equivalent to the DirectX 12 version in MSAAResolve-DX12.

## What it does

The test renders a fullscreen triangle that generates random black/white pixels based on pixel coordinates and frame count, using 8x MSAA (8 samples per pixel). It then performs an MSAA resolve operation to combine the 8 samples into a single sample per pixel and displays the result.

## Features

- **X11 Native Window**: Uses X11 directly for windowing (no GLFW, no Wayland)
- **8x MSAA**: Creates MSAA render targets with 8 samples per pixel
- **4K Resolution**: Renders at 3840x2160 resolution
- **MSAA Resolve**: Demonstrates OpenGL's MSAA resolve using glBlitFramebuffer
- **Random Pattern**: Generates animated random black/white pixel pattern
- **GLAD Integration**: Uses GLAD for dynamic OpenGL function loading
- **No External Dependencies**: Only uses system libraries (X11, OpenGL) + GLAD (included)

## Requirements

- Linux system with X11
- OpenGL 3.3+ compatible graphics driver  
- Development packages: build-essential, libx11-dev, libgl1-mesa-dev, mesa-common-dev, libglu1-mesa-dev
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
g++ -std=c++17 -O2 -Wall -Wextra -o _out/MSAAResolve-GLVK _out/main.o _out/glad.o -lX11 -lGL -ldl
```

## Running

```bash
./_out/MSAAResolve-GLVK
```

Controls:
- **ESC**: Exit the application
- **Close button**: Exit the application

## Technical Details

### OpenGL Function Loading
- Uses GLAD (glad.c/glad.h) for dynamic loading of OpenGL functions
- Supports OpenGL 4.6 compatibility profile with extensions
- Initializes after OpenGL context creation with `gladLoadGL()`

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
├── main.cpp              # Main application with X11/OpenGL code
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

The application creates a 4K window and renders animated random patterns using 8x MSAA, demonstrating OpenGL's multisampling capabilities equivalent to the DirectX 12 version.

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
- Ensure you have X11 running (not Wayland-only)
- Graphics drivers must support OpenGL 3.3+
- For remote connections, make sure X11 forwarding is enabled
