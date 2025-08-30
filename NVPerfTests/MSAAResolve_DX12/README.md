# DirectX 12 MSAA Resolve Demo

This is a DirectX 12 program that demonstrates MSAA (Multi-Sample Anti-Aliasing) resolve operations. The program creates an 8x MSAA render target and resolves it to a regular texture every frame using `ResolveSubresource`.

## Features

- DirectX 12 initialization with hardware adapter selection
- 8x MSAA render target creation (1920x1080)
- Regular texture for resolve destination
- Per-frame MSAA resolve operation using `ResolveSubresource`
- Proper resource state transitions and synchronization
- Simple main loop with frame rendering

## Requirements

- Windows 10/11 with DirectX 12 support
- Visual Studio 2019/2022 with C++ development tools
- DirectX 12 compatible graphics card

## Build Instructions

### Using Visual Studio

1. Open `DX12MSAAResolve.vcxproj` in Visual Studio
2. Set the platform to x64 (DirectX 12 requires 64-bit)
3. Build the solution (Ctrl+Shift+B)
4. Run the program (F5 or Ctrl+F5)

### Using MSBuild (Command Line)

```bash
msbuild DX12MSAAResolve.vcxproj /p:Configuration=Release /p:Platform=x64
```

## How It Works

The program performs the following operations every frame:

1. **Clear MSAA Target**: Clears the 8x MSAA render target with a blue color
2. **Resource Barriers**: Transitions the MSAA target to `D3D12_RESOURCE_STATE_RESOLVE_SOURCE`
3. **Resolve Operation**: Calls `ResolveSubresource` to resolve from the 8x MSAA texture to a regular texture
4. **Copy to Back Buffer**: Copies the resolved texture to the swap chain back buffer for presentation
5. **State Transitions**: Properly transitions all resources back to their initial states

## Key DirectX 12 Concepts Demonstrated

- **MSAA Resource Creation**: Creating textures with sample count > 1
- **ResolveSubresource**: Converting MSAA textures to regular textures
- **Resource State Management**: Proper use of resource barriers for state transitions
- **Command List Recording**: Recording GPU commands for execution
- **Synchronization**: Using fences to synchronize CPU and GPU work

## File Structure

- `main.cpp` - Main program with DirectX 12 initialization and rendering loop
- `d3dx12.h` - DirectX 12 helper structures and functions
- `DX12MSAAResolve.vcxproj` - Visual Studio project file
- `README.md` - This documentation file

## Performance Notes

The program performs a resolve operation every frame, which demonstrates the API usage but may not be optimal for real applications. In practice, you would typically:

- Only resolve when needed (e.g., when copying to a texture for post-processing)
- Use multiple command lists and queues for better GPU utilization
- Implement proper frame buffering with multiple command allocators

## Troubleshooting

- **"Failed to initialize DirectX 12"**: Ensure your system supports DirectX 12 and you have compatible drivers
- **Compilation errors**: Make sure you have the Windows 10 SDK installed
- **Black screen**: Check if the debug layer is enabled and look for validation errors in the output 