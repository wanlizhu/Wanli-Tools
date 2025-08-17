#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <iostream>
#include <vector>
#include "d3dx12.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class MSAAResolve_DX12 {
private:
    static const UINT FRAME_COUNT = 2;
    static const UINT TEXTURE_WIDTH = 3840;
    static const UINT TEXTURE_HEIGHT = 2160;
    static const UINT MSAA_SAMPLE_COUNT = 8;

    // Pipeline objects
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // MSAA and resolve resources
    ComPtr<ID3D12Resource> m_msaaRenderTarget;
    ComPtr<ID3D12Resource> m_resolveTarget;
    
    // Shader resources
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12Resource> m_constantBuffer;
    UINT8* m_pConstantBufferDataBegin;
    
    // Synchronization objects
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    UINT m_rtvDescriptorSize;

public:
    MSAAResolve_DX12() : m_frameIndex(0), m_fenceValue(0), m_pConstantBufferDataBegin(nullptr) {}

    bool Initialize(HWND hwnd) {
        // Enable the D3D12 debug layer
#if defined(_DEBUG)
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
        }
#endif

        // Create DXGI factory
        ComPtr<IDXGIFactory4> factory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
            return false;
        }

        // Create device
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);
        
        if (FAILED(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)))) {
            return false;
        }

        // Create command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        
        if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)))) {
            return false;
        }

        // Create swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FRAME_COUNT;
        swapChainDesc.Width = TEXTURE_WIDTH;
        swapChainDesc.Height = TEXTURE_HEIGHT;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        if (FAILED(factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain))) {
            return false;
        }

        if (FAILED(swapChain.As(&m_swapChain))) {
            return false;
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT + 2; // +2 for MSAA and resolve targets
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        
        if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)))) {
            return false;
        }

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create frame resources
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT n = 0; n < FRAME_COUNT; n++) {
            if (FAILED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])))) {
                return false;
            }
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }

        // Create MSAA render target
        D3D12_RESOURCE_DESC msaaDesc = {};
        msaaDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        msaaDesc.Width = TEXTURE_WIDTH;
        msaaDesc.Height = TEXTURE_HEIGHT;
        msaaDesc.DepthOrArraySize = 1;
        msaaDesc.MipLevels = 1;
        msaaDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        msaaDesc.SampleDesc.Count = MSAA_SAMPLE_COUNT;
        msaaDesc.SampleDesc.Quality = 0;
        msaaDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        msaaDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE msaaClearValue = {};
        msaaClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        msaaClearValue.Color[0] = 0.0f;
        msaaClearValue.Color[1] = 0.0f;
        msaaClearValue.Color[2] = 0.0f;
        msaaClearValue.Color[3] = 1.0f;

        CD3DX12_HEAP_PROPERTIES msaaHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        if (FAILED(m_device->CreateCommittedResource(
            &msaaHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &msaaDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &msaaClearValue,
            IID_PPV_ARGS(&m_msaaRenderTarget)))) {
            return false;
        }

        // Create RTV for MSAA target
        m_device->CreateRenderTargetView(m_msaaRenderTarget.Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        // Create resolve target (non-MSAA)
        D3D12_RESOURCE_DESC resolveDesc = msaaDesc;
        resolveDesc.SampleDesc.Count = 1;
        resolveDesc.SampleDesc.Quality = 0;

        CD3DX12_HEAP_PROPERTIES resolveHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        if (FAILED(m_device->CreateCommittedResource(
            &resolveHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &resolveDesc,
            D3D12_RESOURCE_STATE_RESOLVE_DEST,
            nullptr,
            IID_PPV_ARGS(&m_resolveTarget)))) {
            return false;
        }

        // Create RTV for resolve target
        m_device->CreateRenderTargetView(m_resolveTarget.Get(), nullptr, rtvHandle);

        // Create command allocator
        if (FAILED(m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocator)))) {
            return false;
        }

        // Create command list
        if (FAILED(m_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_commandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&m_commandList)))) {
            return false;
        }

        m_commandList->Close();

        // Create root signature
        if (!CreateRootSignature()) {
            OutputDebugStringA("Failed to create root signature\n");
            return false;
        }

        // Create pipeline state
        if (!CreatePipelineState()) {
            OutputDebugStringA("Failed to create pipeline state\n");
            return false;
        }

        // Create constant buffer
        if (!CreateConstantBuffer()) {
            OutputDebugStringA("Failed to create constant buffer\n");
            return false;
        }

        // Create synchronization objects
        if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) {
            return false;
        }
        m_fenceValue = 1;

        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr) {
            return false;
        }

        return true;
    }

    void Update() {
        // Simple color animation for demonstration
        static float time = 0.0f;
        time += 0.016f; // ~60 FPS
    }

    void Render() {
        // Record all the commands we need to render the scene into the command list
        PopulateCommandList();

        // Execute the command list
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame
        HRESULT hr = m_swapChain->Present(1, 0);
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to present\n");
        }

        WaitForPreviousFrame();
    }

private:
    bool CreateRootSignature() {
        D3D12_ROOT_PARAMETER rootParameter = {};
        rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter.Descriptor.ShaderRegister = 0;
        rootParameter.Descriptor.RegisterSpace = 0;
        rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 1;
        rootSignatureDesc.pParameters = &rootParameter;
        rootSignatureDesc.NumStaticSamplers = 0;
        rootSignatureDesc.pStaticSamplers = nullptr;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(hr)) {
            if (error) {
                OutputDebugStringA(reinterpret_cast<char*>(error->GetBufferPointer()));
            }
            return false;
        }

        hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
        return SUCCEEDED(hr);
    }

    bool CreatePipelineState() {
        // Compile shaders
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> error;

        OutputDebugStringA("Attempting to compile vertex shader...\n");
        HRESULT hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, &error);
        if (FAILED(hr)) {
            OutputDebugStringA("Vertex shader compilation failed!\n");
            if (error) {
                OutputDebugStringA("Vertex shader error: ");
                OutputDebugStringA(reinterpret_cast<char*>(error->GetBufferPointer()));
                OutputDebugStringA("\n");
            }
            return false;
        }
        OutputDebugStringA("Vertex shader compiled successfully\n");

        OutputDebugStringA("Attempting to compile pixel shader...\n");
        hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, &error);
        if (FAILED(hr)) {
            OutputDebugStringA("Pixel shader compilation failed!\n");
            if (error) {
                OutputDebugStringA("Pixel shader error: ");
                OutputDebugStringA(reinterpret_cast<char*>(error->GetBufferPointer()));
                OutputDebugStringA("\n");
            }
            return false;
        }
        OutputDebugStringA("Pixel shader compiled successfully\n");

        // Create pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { nullptr, 0 }; // No input layout needed
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
        psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = MSAA_SAMPLE_COUNT;
        psoDesc.SampleDesc.Quality = 0;

        OutputDebugStringA("Creating graphics pipeline state...\n");
        hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
        if (FAILED(hr)) {
            OutputDebugStringA("Graphics pipeline state creation failed!\n");
            return false;
        }
        OutputDebugStringA("Graphics pipeline state created successfully\n");
        return true;
    }

    bool CreateConstantBuffer() {
        UINT constantBufferSize = sizeof(UINT) * 4; // frameCount + padding
        
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
        
        HRESULT hr = m_device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer));
        
        if (FAILED(hr)) return false;

        // Map the constant buffer
        CD3DX12_RANGE readRange(0, 0);
        hr = m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBufferDataBegin));
        return SUCCEEDED(hr);
    }

    void PopulateCommandList() {
        // Reset command allocator and command list
        HRESULT hr = m_commandAllocator->Reset();
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to reset command allocator\n");
            return;
        }

        hr = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to reset command list\n");
            return;
        }

        // Render using shader
        CD3DX12_CPU_DESCRIPTOR_HANDLE msaaRtvHandle(
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            FRAME_COUNT, // Skip swap chain RTVs
            m_rtvDescriptorSize);

        // Clear to black first
        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_commandList->ClearRenderTargetView(msaaRtvHandle, clearColor, 0, nullptr);

        // Update constant buffer with frame count
        static UINT frameCount = 0;
        frameCount++;
        UINT constantBufferData[4] = { frameCount, 0, 0, 0 };
        memcpy(m_pConstantBufferDataBegin, &constantBufferData, sizeof(constantBufferData));

        // Set render target
        m_commandList->OMSetRenderTargets(1, &msaaRtvHandle, FALSE, nullptr);

        // Set viewport and scissor rect
        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<float>(TEXTURE_WIDTH);
        viewport.Height = static_cast<float>(TEXTURE_HEIGHT);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        m_commandList->RSSetViewports(1, &viewport);

        D3D12_RECT scissorRect = {};
        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = TEXTURE_WIDTH;
        scissorRect.bottom = TEXTURE_HEIGHT;
        m_commandList->RSSetScissorRects(1, &scissorRect);

        // Set pipeline state and draw fullscreen triangle
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        m_commandList->SetPipelineState(m_pipelineState.Get());
        m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->DrawInstanced(3, 1, 0, 0); // Fullscreen triangle

        // Transition MSAA target to resolve source
        CD3DX12_RESOURCE_BARRIER msaaBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_msaaRenderTarget.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
        m_commandList->ResourceBarrier(1, &msaaBarrier);

        // Perform the MSAA resolve operation
        m_commandList->ResolveSubresource(
            m_resolveTarget.Get(),      // Destination (non-MSAA)
            0,                          // Destination subresource
            m_msaaRenderTarget.Get(),   // Source (MSAA)
            0,                          // Source subresource
            DXGI_FORMAT_R8G8B8A8_UNORM);

        // Transition resolve target to copy source for presenting
        CD3DX12_RESOURCE_BARRIER resolveBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_resolveTarget.Get(),
            D3D12_RESOURCE_STATE_RESOLVE_DEST,
            D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_commandList->ResourceBarrier(1, &resolveBarrier);

        // Transition back buffer to copy destination
        CD3DX12_RESOURCE_BARRIER backBufferBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandList->ResourceBarrier(1, &backBufferBarrier1);

        // Copy resolved texture to back buffer for presentation
        D3D12_TEXTURE_COPY_LOCATION dest = {};
        dest.pResource = m_renderTargets[m_frameIndex].Get();
        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = m_resolveTarget.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0;

        m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

        // Transition back buffer back to present state
        CD3DX12_RESOURCE_BARRIER backBufferBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &backBufferBarrier2);

        // Transition MSAA target back to render target for next frame
        CD3DX12_RESOURCE_BARRIER msaaBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
            m_msaaRenderTarget.Get(),
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &msaaBarrier2);

        // Transition resolve target back to resolve destination for next frame
        CD3DX12_RESOURCE_BARRIER resolveBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
            m_resolveTarget.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_RESOLVE_DEST);
        m_commandList->ResourceBarrier(1, &resolveBarrier2);

        hr = m_commandList->Close();
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to close command list\n");
            return;
        }
    }

    void WaitForPreviousFrame() {
        // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
        // sample illustrates how to use fences for efficient resource usage and to
        // maximize GPU utilization.

        // Signal and increment the fence value.
        const UINT64 fence = m_fenceValue;
        m_commandQueue->Signal(m_fence.Get(), fence);
        m_fenceValue++;

        // Wait until the previous frame is finished.
        if (m_fence->GetCompletedValue() < fence) {
            m_fence->SetEventOnCompletion(fence, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    }

    void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter) {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;
        ComPtr<IDXGIFactory6> factory6;

        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
            for (UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr) {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach();
    }
};

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"DX12MSAAResolveWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window
    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"DirectX 12 MSAA Resolve Demo - 4K",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        3840, 2160,
        nullptr, nullptr, hInstance, nullptr
    );

    if (hWnd == nullptr) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);

    // Initialize DirectX 12
    MSAAResolve_DX12 dx12App;
    if (!dx12App.Initialize(hWnd)) {
        std::cerr << "Failed to initialize DirectX 12" << std::endl;
        return -1;
    }

    // Main loop
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            dx12App.Update();
            dx12App.Render();
        }
    }

    return 0;
}
