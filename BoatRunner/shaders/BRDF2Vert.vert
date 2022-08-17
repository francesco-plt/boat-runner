#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;	// world-view-projection matrix
	mat4 mMat;		// world matrix -> transform the position in world space
	mat4 nMat;		// matrix to transform normal vector directions in world space
} ubo;

/*	
	inPosition:
	set of vec3 normalized screen coordinates to denote the
	position of the elements to connect
*/
layout(location = 0) in vec3 inPosition;	// position of the vertex in local coordinates
layout(location = 1) in vec3 inNormal;		// direction of the normal vector in local space
layout(location = 2) in vec2 inTexCoord;	// UV coordinates of the vertex.

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;

void main() {
	gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);	// clipping coordinates
	fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz;	//pixel position, world coordinates
	fragNorm = (ubo.nMat * vec4(inNormal, 0.0)).xyz;	//normal, world coordinates
	fragUV = inTexCoord;								//UV coordinates
}