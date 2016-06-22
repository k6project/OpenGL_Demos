#version 410

uniform mat4 uProjection;
uniform mat4 uView;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in mat4 aToWorld;
layout(location = 6) in uint aColorMod;

out float vLighting;
flat out uint vColorModifier;

const float AMBIENT = 0.2;
const vec3 LIGHT_POS = vec3(0);

void main()
{ 
    vec4 LocalToView = uView * aToWorld * vec4(aPosition, 1.0);
    gl_Position = uProjection * LocalToView;
    vec3 LightSrc = LIGHT_POS - LocalToView.xyz;
    vec3 Normal = (uView * aToWorld * vec4(aNormal, 0.0)).xyz;
    float Diffuse = max(0.0, dot(normalize(LightSrc), Normal)); 
    vLighting = Diffuse + AMBIENT;
    vColorModifier = aColorMod;
}
