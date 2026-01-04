#version 460

layout (set = 1, binding = 0) uniform sampler2D imgSampler;
layout (location = 0) in vec2 texCoords;
layout (location = 0) out vec4 outColor;

void main() {
	outColor = texture(imgSampler, texCoords);
}
