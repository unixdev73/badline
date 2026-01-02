#version 460

layout (std430, push_constant) uniform pushConstant {
	mat4 camProj;
	mat4 camView;
};

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform modelMats {
	mat4 model[12];
};

void main() {
	gl_Position = camProj * camView * model[gl_InstanceIndex] * vec4(vertexPos, 1);
	outColor = inColor;
}
