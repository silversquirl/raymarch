#version 330 core
layout(location = 0) in vec2 vert;
out vec2 coord;

void main() {
	gl_Position = vec4(vert, 0, 1);
	coord = vert;
}
