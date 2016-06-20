#version 410

uniform mat4 uProjection;
uniform mat4 uView;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in mat4 aToWorld;
layout(location = 6) in uint aColorMod;

out vec3 vNormal;
flat out uint vColorModifier;

void main()
{ 
    gl_Position = uProjection * uView * aToWorld * vec4(aPosition, 1.0);
    vNormal = abs(aNormal);
    vColorModifier = aColorMod;
}
