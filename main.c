#include "glad.h"
#include "v.h"
#include "vgl.h"

int main() {
	GLFWwindow *win = vgl_init(1024, 1024, "Ray Marcher");
	if (!win) panic("Window creation failed");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLfloat verts[] = {
		1, 1,
		1, -1,
		-1, 1,
		-1, -1,
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

	GLuint shader = vgl_shader_file("vertex.glsl", "fragment.glsl");
	if (!shader) panic("Shader compilation failed");
	glUseProgram(shader);

	while (!(glfwPollEvents(), glfwWindowShouldClose(win))) {
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(0);

		glfwSwapBuffers(win);
	}

	return 0;
}
