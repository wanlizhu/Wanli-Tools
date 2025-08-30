#version 430 core

layout(std430, binding = 0) writeonly buffer StorageData {
    vec4 data[];
} storageData;

uniform uint frameIndex;

in vec2 uv;
out vec4 fragColor;

uint hash(uint x)
{
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}

float random(uint seed)
{
    return float(hash(seed)) / 4294967296.0;
}

void main()
{
    uint pixelIdx = uint(gl_FragCoord.y) * 3840u + uint(gl_FragCoord.x);
    
    for(int i = 0; i < 64; i++) {
        uint seed = pixelIdx + frameIndex * 12345u + uint(i) * 7919u;
        uint index = hash(seed) % (storageData.data.length());
        
        vec4 randColor;
        randColor.r = random(seed);
        randColor.g = random(hash(seed));
        randColor.b = random(hash(hash(seed)));
        randColor.a = random(hash(hash(hash(seed))));
        
        storageData.data[index] = randColor;
    }
    
    if(uint(gl_FragCoord.x) == 100u && uint(gl_FragCoord.y) == 100u) {
        uint seed = pixelIdx + frameIndex * 12345u;
        fragColor = vec4(random(seed), random(hash(seed)), random(hash(hash(seed))), 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
