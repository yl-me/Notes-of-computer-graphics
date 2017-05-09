// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <map>
#include <iostream>
#include <cmath>
#include <SOIL\SOIL.h>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

const GLuint WIDTH = 800, HEIGHT = 600;

GLfloat deltaTime = 0.0f;       // 每帧所用时间
GLfloat lastFrame = 0.0f;       // 下一帧的时间

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

GLfloat lastX = 400;        // 屏幕的中心位置
GLfloat lastY = 300;

GLboolean firstMouse = true;   // 第一次鼠标位置是否正确

bool keys[1024];

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);  // 键盘
void mouse_callback(GLFWwindow * window, double xpos, double ypos);           // 鼠标
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);    // 滑轮
void do_movement();

GLuint loadTexture(const GLchar * path, GLboolean alpha = false);
GLuint loadCubemapTexture(std::vector<const char *> texture_face, GLboolean alpha = false);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);      // opengl主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);      // opengl副版本号
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);     // 设置为核心模式
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);           // 不能调整窗口大小
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow * window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);     // 键盘回调函数

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    // 鼠标模式

	glfwSetCursorPosCallback(window, mouse_callback);    // 鼠标回调函数

	glfwSetScrollCallback(window, scroll_callback);      // 滚轮回调函数

	glewExperimental = GL_TRUE;          // 设置为GL_TRUE是为了更好的使用核心模式
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);
	Shader shader("path/base.vert", "path/base.frag");
	Shader normalShader("path/normal.vert", "path/normal.frag", "path/pass_through.geom");
	Model mModel("path/nanosuit/nanosuit.obj");

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 100.0f);
	shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	normalShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	while (!glfwWindowShouldClose(window)) {   // 检查GLFW是否被要求退出
		glfwPollEvents();                      // 检查是否触发事件，来调用回调函数
		do_movement();

		GLfloat currentFrame = glfwGetTime();    // 当前帧的时间
		deltaTime = currentFrame - lastFrame;    // 每帧所用时间
		lastFrame = currentFrame;

//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		glm::mat4 model;
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		mModel.Draw(shader);

		normalShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		mModel.Draw(normalShader);

		glfwSwapBuffers(window);               // 颜色双缓冲
	}

	glfwTerminate();
	return 0;
}

GLuint loadTexture(const GLchar * path, GLboolean alpha)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char * image = SOIL_load_image(path, &width, &height, 0, alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, alpha ? GL_MIRRORED_REPEAT : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, alpha ? GL_MIRRORED_REPEAT : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}

GLuint loadCubemapTexture(std::vector<const char *> texture_face, GLboolean alpha)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height;
	int size = texture_face.size();
	for (int i = 0; i < size; ++i) {
		unsigned char * image = SOIL_load_image(texture_face[i], &width, &height, 0, alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return textureID;
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_TRUE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key >= 1 && key <= 1024) {
		if (action == GLFW_PRESS) {
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			keys[key] = false;
		}
	}
}

void do_movement()
{
	if (keys[GLFW_KEY_W]) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (keys[GLFW_KEY_S]) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (keys[GLFW_KEY_A]) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (keys[GLFW_KEY_D]) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset * 0.01f);
}