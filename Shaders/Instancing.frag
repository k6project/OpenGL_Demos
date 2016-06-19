#version 410

const vec3 Colors[2] = vec3[](
    vec3(0.0), vec3(1.0)
);

layout(location = 0)out vec4 outColor;

in vec3 vNormal;
flat in uint vColorModifier;

void main()
{
    vec3 outRGB = abs(Colors[vColorModifier] - vNormal);
    outColor = vec4(outRGB, 1.0);
}
