struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer FrameConstants : register(b0)
{
    uint frameIndex;
    uint3 padding;
};

// Simple hash function for random generation
uint hash(uint x)
{
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}

// Generate random float between 0 and 1
float random(uint2 pixelCoord, uint frame)
{
    uint seed = pixelCoord.x + pixelCoord.y * 3840u + frame * 12345u;
    return float(hash(seed)) / 4294967296.0; // Divide by 2^32
}

float4 PSMain(PSInput input) : SV_TARGET
{
    uint2 pixelCoord = uint2(input.position.xy);
    float randValue = random(pixelCoord, frameIndex);
    
    // Generate 0 or 1 based on random value
    float result = randValue > 0.5 ? 1.0 : 0.0;
    
    return float4(result, result, result, 1.0);
} 