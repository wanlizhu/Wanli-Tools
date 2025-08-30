#version 430 core

uniform samplerCube texCube;
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
        float x = float(hash(seed)) / 2147483648.0 - 1.0;
        seed = hash(seed);
        float y = float(hash(seed)) / 2147483648.0 - 1.0;
        seed = hash(seed);
        float z = float(hash(seed)) / 2147483648.0 - 1.0;
        vec3 dir = normalize(vec3(x, y, z));
        color += texture(texCube, dir);
    }
    
    color = color / 64.0;
    
    if(uint(gl_FragCoord.x) == 100u && uint(gl_FragCoord.y) == 100u) {
        fragColor = color;
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
