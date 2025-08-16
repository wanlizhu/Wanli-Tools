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
- Development packages: libx11-dev, libgl1-mesa-dev, build-essential

## Building

### Ubuntu/Debian
```bash
# Install dependencies
make install-deps

# Build the project
make
```

### Red Hat/Fedora/CentOS
```bash
# Install dependencies
make install-deps-rpm

# Build the project
make
```

### Manual build
```bash
g++ -std=c++17 -O2 -Wall -Wextra -I/usr/include/X11 -o MSAAResolve-GLVK main.cpp -lX11 -lGL -lGLX
```

## Running

```bash
./MSAAResolve-GLVK
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
