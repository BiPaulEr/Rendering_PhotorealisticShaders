#version 450 core // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition; // The 1st input attribute is the position (CPU side: glVertexAttrib 0)
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoord;

uniform mat4 projectionMat, modelViewMat, normalMat; 

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoord;

void main() {
	vec4 p = modelViewMat * vec4 (vPosition, 1.0);
    gl_Position =  projectionMat * p; // mandatory to fire rasterization properly
    vec4 n = normalMat * vec4 (vNormal, 1.0);
    fPosition = p.xyz;
    fNormal = normalize (n.xyz);
    fTexCoord = vTexCoord;
}