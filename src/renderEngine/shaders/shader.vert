#version 460

layout (location = 0) in vec4 vertexPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() {
	gl_Position = vertexPosition;
	outColor = inColor;
}
