#version 460

layout (std430, push_constant) uniform pushConstant {
	mat4 objModel;
	mat4 camView;
};

layout (location = 0) in vec4 quad0; // posX, posY, posZ, texX
layout (location = 1) in vec4 quad1; // texY, norX, norY, norZ
layout (location = 2) in vec4 quad2; // colR, colG, colB, colA

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec4 outColor;

layout (set = 0, binding = 0) uniform camData {
	mat4 camProj;
};

void main() {
	gl_Position = camProj * camView * objModel * vec4(quad0.xyz, 1);
	outTexCoord = vec2(quad0.w, quad1.x);
	vec3 normal = vec3(quad1.yzw);
	outColor = quad2;
}
