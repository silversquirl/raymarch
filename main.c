#include "glad.h"
#include "v.h"
#include "vgl.h"

struct state {
	float scale[2];
	_Bool capture;
	double mx, my;

	struct {
		vec3 pos;
		vec3 vel;
		quat look;
	} cam;
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

static void mbutton_cb(GLFWwindow *win, int btn, int act, int mods) {
	struct state *state = glfwGetWindowUserPointer(win);

	if (btn == GLFW_MOUSE_BUTTON_LEFT && act == GLFW_PRESS) {
		state->capture = !state->capture;
		if (state->capture) {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwGetCursorPos(win, &state->mx, &state->my);
		} else {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

const double ROT_SCALE = 0.001;
static void mmove_cb(GLFWwindow *win, double x, double y) {
	struct state *state = glfwGetWindowUserPointer(win);
	if (!state->capture) return;

	double dx = x - state->mx;
	double dy = y - state->my;
	state->mx = x;
	state->my = y;

	dx *= ROT_SCALE;
	dy *= ROT_SCALE;

	quat rot = qeuler(vec3(-dy, -dx, 0), VGL_XYZ);
	state->cam.look = qmul(state->cam.look, rot);
}

const GLfloat VELOCITY = 2.0f;
static void key_cb(GLFWwindow *win, int key, int scan, int act, int mods) {
	struct state *state = glfwGetWindowUserPointer(win);
	if (!state->capture) return;

	GLfloat delta = VELOCITY;
	switch (act) {
	case GLFW_PRESS:
		break;

	case GLFW_RELEASE:
		delta = -delta;
		break;

	default:
		return;
	}

	switch (key) {
	case GLFW_KEY_W:
		state->cam.vel.z -= delta;
		break;

	case GLFW_KEY_S:
		state->cam.vel.z += delta;
		break;

	case GLFW_KEY_A:
		state->cam.vel.x -= delta;
		break;

	case GLFW_KEY_D:
		state->cam.vel.x += delta;
		break;
	}
}

int main() {
	int width = 1280, height = 1024;
	GLFWwindow *win = vgl_init(width, height, "Ray Marcher", .resizable = 1);
	if (!win) panic("Window creation failed");

	struct state state = {0};
	glfwSetWindowUserPointer(win, &state);
	state.cam.pos = vec3(0, 0, 20);
	state.cam.look = quat(1, 0, 0, 0);
	resize_cb(win, width, height);

	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	glfwSetFramebufferSizeCallback(win, resize_cb);
	glfwSetMouseButtonCallback(win, mbutton_cb);
	glfwSetCursorPosCallback(win, mmove_cb);
	glfwSetKeyCallback(win, key_cb);

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
	GLuint u_cam_pos = glGetUniformLocation(shader, "cam_pos");
	GLuint u_cam_look = glGetUniformLocation(shader, "cam_look");

	glfwSwapInterval(0);
	glfwSetTime(0);
	double time = 0;
	int frames = 0;
	while (!(glfwPollEvents(), glfwWindowShouldClose(win))) {
		glUniform2fv(u_scale, 1, state.scale);
		glUniform3fv(u_cam_pos, 1, state.cam.pos.v);
		glUniform4fv(u_cam_look, 1, state.cam.look.q);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(0);

		glfwSwapBuffers(win);

		double dt = glfwGetTime();
		glfwSetTime(0);

		vec3 vel = v3rot(state.cam.vel, state.cam.look);
		vel = v3sop(vel, *, dt);
		state.cam.pos = v3v3op(state.cam.pos, +, vel);

		time += dt;
		frames++;
		if (time >= 1) {
			printf(u8"%.5g \u03bcs/frame (%.1ffps)\n", time/frames * 1e6, frames/time);
			time = 0;
			frames = 0;
		}
	}

	return 0;
}
