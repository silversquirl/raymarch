#include "glad.h"
#include "v.h"
#include "vgl.h"

struct state {
	float scale[2];
};

static void resize_cb(GLFWwindow *win, int w, int h) {
	struct state *state = glfwGetWindowUserPointer(win);

	if (h > w) {
		state->scale[0] = 1.0f;
		state->scale[1] = (float)h / (float)w;
	} else {
		state->scale[0] = (float)w / (float)h;
		state->scale[1] = 1.0f;
	}

	glViewport(0, 0, w, h);
}

int main() {
	int width = 1280, height = 1024;
	GLFWwindow *win = vgl_init(width, height, "Ray Marcher", .resizable = 1);
	if (!win) panic("Window creation failed");

	struct state state;
	glfwSetWindowUserPointer(win, &state);

	resize_cb(win, width, height);
	glfwSetFramebufferSizeCallback(win, resize_cb);

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

	GLuint u_scale = glGetUniformLocation(shader, "scale");

	glfwSwapInterval(0);
	glfwSetTime(0);
	int frames = 0;
	while (!(glfwPollEvents(), glfwWindowShouldClose(win))) {
		glUniform2fv(u_scale, 1, state.scale);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(0);

		glfwSwapBuffers(win);

		frames++;
		double time = glfwGetTime();
		if (time >= 1) {
			printf("%.3g ms/frame (%.1ffps)\n", time/frames, frames/time);
			glfwSetTime(0);
			frames = 0;
		}
	}

	return 0;
}
