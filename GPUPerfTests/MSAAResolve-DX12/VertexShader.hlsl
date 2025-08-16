struct VSInput
{
    uint vertexID : SV_VertexID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    // Simple fullscreen triangle generation
    // Vertex 0: (-1, -1, 0, 1)
    // Vertex 1: (-1,  3, 0, 1)  
    // Vertex 2: ( 3, -1, 0, 1)
    
    if (input.vertexID == 0) {
        result.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        result.uv = float2(0.0f, 1.0f);
    }
    else if (input.vertexID == 1) {
        result.position = float4(-1.0f, 3.0f, 0.0f, 1.0f);
        result.uv = float2(0.0f, -1.0f);
    }
    else { // vertexID == 2
        result.position = float4(3.0f, -1.0f, 0.0f, 1.0f);
        result.uv = float2(2.0f, 1.0f);
    }
    
    return result;
} 