#version 460

layout (std430, push_constant) uniform pushConstant {
	mat4 objModel;
	mat4 camView;
};

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec4 outColor;

layout (set = 0, binding = 0) uniform camData {
	mat4 camProj;
};

void main() {
	gl_Position = camProj * camView * objModel * vec4(vertexPos, 1);
	outTexCoord = inTexCoord;
	outColor = inColor;
}
