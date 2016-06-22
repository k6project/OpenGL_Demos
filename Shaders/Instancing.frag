#version 410

const vec4 COLORS[3] = vec4[](
    vec4(1.0, 0.0, 0.0, 1.0), 
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
);

layout(location = 0)out vec4 outColor;

in float vLighting;
flat in uint vColorModifier;

void main()
{
    outColor = vLighting * COLORS[vColorModifier];
}
