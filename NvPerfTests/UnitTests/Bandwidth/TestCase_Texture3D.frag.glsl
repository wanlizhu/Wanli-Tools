#version 430 core

uniform sampler3D tex3D;
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

void main()
{
    vec4 color = vec4(0.0);
    
    for(int i = 0; i < 64; i++) {
        uint seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 3840u + frameIndex * 12345u + uint(i);
        float u = float(hash(seed)) / 4294967296.0;
        seed = hash(seed);
        float v = float(hash(seed)) / 4294967296.0;
        seed = hash(seed);
        float w = float(hash(seed)) / 4294967296.0;
        color += texture(tex3D, vec3(u, v, w));
    }
    
    color = color / 64.0;
    
    if(uint(gl_FragCoord.x) == 100u && uint(gl_FragCoord.y) == 100u) {
        fragColor = color;
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
