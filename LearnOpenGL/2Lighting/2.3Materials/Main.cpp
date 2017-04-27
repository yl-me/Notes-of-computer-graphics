// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <SOIL\SOIL.h>
#include "shader.h"
#include "Camera.h"

const GLuint WIDTH = 800, HEIGHT = 600;

GLfloat test = 0.2f;

GLfloat deltaTime = 0.0f;       // 每帧所用时间
GLfloat lastFrame = 0.0f;       // 下一帧的时间

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

GLfloat lastX = 400;        // 屏幕的中心位置
GLfloat lastY = 300;

GLboolean firstMouse = true;   // 第一次鼠标位置是否正确

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

bool keys[1024];

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);  // 键盘
void mouse_callback(GLFWwindow * window, double xpos, double ypos);           // 鼠标
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);    // 滑轮
void do_movement();

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

	Shader ourShader("path/default_vs.txt", "path/default_frag.txt");

	Shader lightShader("path/light_vs.txt", "path/light_frag.txt");

	GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};
	GLfloat lightColor[][10] = {
		{ 0.0215,	0.1745,	0.0215,	0.07568, 0.6142,0.07568, 0.633,	0.727811, 0.633, 0.6 }, // emerald
		{ 0.135, 0.2225, 0.1575, 0.54, 0.89, 0.63, 0.316228, 0.316228, 0.316228, 0.1 }, // jade
		{ 0.05375, 0.05, 0.06625, 0.18275, 0.17, 0.22525, 0.332741, 0.328634, 0.346435, 0.3 },    // obsidian
		{ 0.25, 0.20725, 0.20725, 1, 0.829, 0.829, 0.296648, 0.296648, 0.296648, 0.088 }, // pearl	
		{ 0.1745,	0.01175, 0.01175, 0.61424, 0.04136,	0.04136, 0.727811, 0.626959, 0.626959, 0.6 },    // ruby	
		{ 0.1, 0.18725, 0.1745, 0.396, 0.74151, 0.69102, 0.297254, 0.30829, 0.306678, 0.1 },    // turquoise	
		{ 0.329412, 0.223529, 0.027451, 0.780392, 0.568627, 0.113725, 0.992157, 0.941176, 0.807843, 0.21794872 },   // brass	
		{ 0.2125, 0.1275, 0.054, 0.714, 0.4284, 0.18144, 0.393548, 0.271906, 0.166721, 0.2 },   // bronze	
		{ 0.25, 0.25, 0.25, 0.4, 0.4, 0.4, 0.774597, 0.774597, 0.774597, 0.6 },    // chrome	
		{ 0.19125, 0.0735, 0.0225, 0.7038, 0.27048, 0.0828, 0.256777, 0.137622, 0.086014, 0.1 },    // copper	
		{ 0.24725, 0.1995, 0.0745, 0.75164, 0.60648, 0.22648, 0.628281, 0.555802, 0.366065, 0.4 },    // gold	
		{ 0.19225, 0.19225, 0.19225, 0.50754, 0.50754, 0.50754, 0.508273, 0.508273, 0.508273, 0.4 },    // silver	
		{ 0.0, 0.0, 0.0, 0.01, 0.01, 0.01, 0.50, 0.50, 0.50, 0.25 },    // black plastic	
		{ 0.0, 0.1, 0.06, 0.0, 0.50980392, 0.50980392, 0.50196078, 0.50196078, 0.50196078, 0.25 },    // cyan plastic	
		{ 0.0, 0.0, 0.0, 0.1, 0.35, 0.1, 0.45, 0.55, 0.45, 0.25 },    // green plastic	
		{ 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.7, 0.6, 0.6, 0.25 },    // red plastic	
		{ 0.0, 0.0, 0.0, 0.55, 0.55, 0.55, 0.70, 0.70, 0.70, 0.25 },   // white plastic	
		{ 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.60, 0.60, 0.50, 0.25 },   // yellow plastic	
		{ 0.02, 0.02, 0.02, 0.01, 0.01, 0.01, 0.4, 0.4, 0.4, 0.078125 },   // black rubber	
		{ 0.0, 0.05, 0.05, 0.4, 0.5, 0.5, 0.04, 0.7, 0.7, 0.078125 },    // cyan rubber	
		{ 0.0, 0.05, 0.0, 0.4, 0.5, 0.4, 0.04, 0.7, 0.04, 0.07812 },     // green rubber	
		{ 0.05, 0.0, 0.0, 0.5, 0.4, 0.4, 0.7, 0.04, 0.04, 0.078125 },   // red rubber	
		{ 0.05, 0.05, 0.05, 0.5, 0.5, 0.5, 0.7, 0.7, 0.7, 0.078125 },   // white rubber	
		{ 0.05, 0.05, 0.0, 0.5, 0.5, 0.4, 0.7, 0.7, 0.04, 0.078125 }   // yellow rubber	
	};

	enum Materrial {
		emerald, jade, obsidian, pearl, ruby, turquoise, brass, bronze, chrome, opper,
		gold, silver, black_plastic, cyan_plastic, green_plastic, red_plastic,
		white_plastic, yellow_plastic, black_rubber, cyan_rubber,
		green_rubber, red_rubber, white_rubber, yellow_rubber
	};
	
	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);       // 生成一个VBO对象
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);     // 把VBO绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 把顶点数据复制到缓冲的内存中

	// 位置属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid *)0);   // 解析顶点数据
	glEnableVertexAttribArray(0);           // 启用顶点位置属性

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid *)(3 * sizeof(float)));   // 解析顶点数据
	glEnableVertexAttribArray(1);           // 启用顶点位置属性

	glBindBuffer(GL_ARRAY_BUFFER, 0);  // 可以安全解绑，对于glVertexAttribPointer绑定的VBO对象
	glBindVertexArray(0);  // 解绑VAO


	GLuint lightVAO;           // 灯
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);     // 把VBO绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 把顶点数据复制到缓冲的内存中
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);



	while (!glfwWindowShouldClose(window)) {   // 检查GLFW是否被要求退出
		glfwPollEvents();                      // 检查是否触发事件，来调用回调函数
		do_movement();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLfloat currentFrame = glfwGetTime();    // 当前帧的时间
		deltaTime = currentFrame - lastFrame;    // 每帧所用时间
		lastFrame = currentFrame;

		ourShader.Use();
//		GLint lightColorLoc = glGetUniformLocation(ourShader.Program, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(ourShader.Program, "light.position");
//		lightPos.x = sin(glfwGetTime() / 2.0f);
//		lightPos.y = sqrt(1 - lightPos.x * lightPos.x);
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
//		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);    // 白色

		GLint cameraPosLoc = glGetUniformLocation(ourShader.Program, "cameraPos");
		glUniform3f(cameraPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		GLint matAmbientLoc = glGetUniformLocation(ourShader.Program, "material.ambient");
		GLint matDiffuseLoc = glGetUniformLocation(ourShader.Program, "material.diffuse");
		GLint matSpecularLoc = glGetUniformLocation(ourShader.Program, "material.specular");
		GLint matShininessLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
		glUniform3f(matAmbientLoc, lightColor[emerald][0], lightColor[emerald][1], lightColor[emerald][2]);
		glUniform3f(matDiffuseLoc, lightColor[emerald][3], lightColor[emerald][4], lightColor[emerald][5]);
		glUniform3f(matSpecularLoc, lightColor[emerald][6], lightColor[emerald][7], lightColor[emerald][8]);
		glUniform1f(matShininessLoc, 32.0f);

		GLint lightAmbientLoc = glGetUniformLocation(ourShader.Program, "light.ambient");
		GLint lightDiffuseLoc = glGetUniformLocation(ourShader.Program, "light.diffuse");
		GLint lightSpecularLoc = glGetUniformLocation(ourShader.Program, "light.specular");
		glUniform3f(lightAmbientLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightDiffuseLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);

	/*	glm::vec3 lightColor;
		lightColor.x = sin(glfwGetTime() * 2.0f);
		lightColor.y = sin(glfwGetTime() * 0.7f);
		lightColor.z = sin(glfwGetTime() * 1.3f);
		glm::vec3 ambientColor = lightColor * 0.5f;
		glm::vec3 DiffuseColor = lightColor * 0.2f;
		glUniform3f(lightAmbientLoc, DiffuseColor.x, DiffuseColor.y, DiffuseColor.z);
		glUniform3f(lightDiffuseLoc, ambientColor.x, ambientColor.y, ambientColor.z);*/
		
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projectionLoc = glGetUniformLocation(ourShader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glm::mat4 model = glm::mat4();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		lightShader.Use();
		modelLoc = glGetUniformLocation(lightShader.Program, "model");
		viewLoc = glGetUniformLocation(lightShader.Program, "view");
		projectionLoc = glGetUniformLocation(lightShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
//		model = glm::rotate(model, 20.0f, glm::vec3(1.0f, 0.3f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glfwSwapBuffers(window);               // 颜色双缓冲
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
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
