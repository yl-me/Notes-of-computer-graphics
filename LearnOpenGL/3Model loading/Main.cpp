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
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

const GLuint WIDTH = 800, HEIGHT = 600;

GLfloat test = 0.2f;

GLfloat deltaTime = 0.0f;       // 每帧所用时间
GLfloat lastFrame = 0.0f;       // 下一帧的时间

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

GLfloat lastX = 400;        // 屏幕的中心位置
GLfloat lastY = 300;

GLboolean firstMouse = true;   // 第一次鼠标位置是否正确

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

glm::vec3 pointLightPos(2.4f, 0.8f, -3.5f);

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

	Shader ourShader("path/default_vs.glsl", "path/Zdefault_frag.glsl");

	Shader lightShader("path/light_vs.glsl", "path/light_frag.glsl");

	GLfloat vertices[] = {
		// Positions           // Normals           // Texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};
	
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);       // 生成一个VBO对象
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);     // 把VBO绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 把顶点数据复制到缓冲的内存中

	// 顶点坐标
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *)0);   // 解析顶点数据
	glEnableVertexAttribArray(0);           // 启用顶点位置属性
	// 法线坐标
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *)(3 * sizeof(float)));   // 解析顶点数据
	glEnableVertexAttribArray(1);           // 启用顶点位置属性

	// 纹理坐标
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *)(6 * sizeof(float)));   // 解析顶点数据
	glEnableVertexAttribArray(2);           // 启用顶点位置属性

	glBindBuffer(GL_ARRAY_BUFFER, 0);  // 可以安全解绑，对于glVertexAttribPointer绑定的VBO对象
	glBindVertexArray(0);  // 解绑VAO


	GLuint lightVAO;           // 灯
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);     // 把VBO绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 把顶点数据复制到缓冲的内存中
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLuint diffuseMap;
	glGenTextures(1, &diffuseMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	int width, height;
	unsigned char * image = SOIL_load_image("path/container2.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);   // 自动生成多级渐远纹理
	glBindTexture(GL_TEXTURE_2D, 0);  // 解绑


	GLuint specularMap;
	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	image = SOIL_load_image("path/container2_specular.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);   // 自动生成多级渐远纹理
	glBindTexture(GL_TEXTURE_2D, 0);  // 解绑


	Model mModel("path/nanosuit/nanosuit.obj");

	while (!glfwWindowShouldClose(window)) {   // 检查GLFW是否被要求退出
		glfwPollEvents();                      // 检查是否触发事件，来调用回调函数
		do_movement();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLfloat currentFrame = glfwGetTime();    // 当前帧的时间
		deltaTime = currentFrame - lastFrame;    // 每帧所用时间
		lastFrame = currentFrame;

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		ourShader.Use();

		GLint cameraPosLoc = glGetUniformLocation(ourShader.Program, "cameraPos");
		glUniform3f(cameraPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		GLint matDiffuseLoc = glGetUniformLocation(ourShader.Program, "material.diffuse");
		GLint matSpecularLoc = glGetUniformLocation(ourShader.Program, "material.specular");
		GLint matShininessLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
		/*************************************************************************************/
		glUniform1i(matDiffuseLoc, 0);
		glUniform1i(matSpecularLoc, 1);
		/*************************************************************************************/
		glUniform1f(matShininessLoc, 32.0f);


		// Direction Light
		GLint lightDirDirLoc = glGetUniformLocation(ourShader.Program, "dirLight.direction");
		glUniform3f(lightDirDirLoc, lightPos.x, lightPos.y, lightPos.z);
		GLint lightDirAmbientLoc = glGetUniformLocation(ourShader.Program, "dirLight.ambient");
		GLint lightDirDiffuseLoc = glGetUniformLocation(ourShader.Program, "dirLight.diffuse");
		GLint lightDirSpecularLoc = glGetUniformLocation(ourShader.Program, "dirLight.specular");
		glUniform3f(lightDirAmbientLoc, 0.2f, 0.2f, 0.2f);
		glUniform3f(lightDirDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightDirSpecularLoc, 1.0f, 1.0f, 1.0f);


		// Point Light
		GLint lightPointDirLoc = glGetUniformLocation(ourShader.Program, "pointLight.direction");
		glUniform3f(lightPointDirLoc, camera.Front.x, camera.Front.y, camera.Front.z);  // pointLight
		GLint lightPointPosLoc = glGetUniformLocation(ourShader.Program, "pointLight.position");
		glUniform3f(lightPointPosLoc, pointLightPos.x, pointLightPos.y, pointLightPos.z);
		GLint lightPointContantLoc = glGetUniformLocation(ourShader.Program, "pointLight.contant");
		GLint lightPointLinearLoc = glGetUniformLocation(ourShader.Program, "pointLight.linear");
		GLint lightPointQuadraticLoc = glGetUniformLocation(ourShader.Program, "pointLight.quadratic");
		glUniform1f(lightPointContantLoc, 1.0f);
		glUniform1f(lightPointLinearLoc, 0.09f);
		glUniform1f(lightPointQuadraticLoc, 0.032f);
		GLint lightPointAmbientLoc = glGetUniformLocation(ourShader.Program, "pointLight.ambient");
		GLint lightPointDiffuseLoc = glGetUniformLocation(ourShader.Program, "pointLight.diffuse");
		GLint lightPointSpecularLoc = glGetUniformLocation(ourShader.Program, "pointLight.specular");
		glUniform3f(lightPointAmbientLoc, 0.2f, 0.2f, 0.2f);
		glUniform3f(lightPointDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightPointSpecularLoc, 1.0f, 1.0f, 1.0f);


		// Spot Light
		GLint lightSpotDirLoc = glGetUniformLocation(ourShader.Program, "spotLight.direction");
		glUniform3f(lightSpotDirLoc, camera.Front.x, camera.Front.y, camera.Front.z);
		GLint lightSpotPosLoc = glGetUniformLocation(ourShader.Program, "spotLight.position");
		glUniform3f(lightSpotPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		GLint lightSpotCutOffLoc = glGetUniformLocation(ourShader.Program, "spotLight.cutOff");
		glUniform1f(lightSpotCutOffLoc, glm::cos(glm::radians(12.5f)));
		GLint lightSpotOuterCutOffLoc = glGetUniformLocation(ourShader.Program, "spotLight.outerCutOff");
		glUniform1f(lightSpotOuterCutOffLoc, glm::cos(glm::radians(22.5f)));
		GLint lightSpotContantLoc = glGetUniformLocation(ourShader.Program, "spotLight.contant");
		GLint lightSpotLinearLoc = glGetUniformLocation(ourShader.Program, "spotLight.linear");
		GLint lightSpotQuadraticLoc = glGetUniformLocation(ourShader.Program, "spotLight.quadratic");
		glUniform1f(lightSpotContantLoc, 1.0f);
		glUniform1f(lightSpotLinearLoc, 0.09f);
		glUniform1f(lightSpotQuadraticLoc, 0.032f);
		GLint lightSpotAmbientLoc = glGetUniformLocation(ourShader.Program, "spotLight.ambient");
		GLint lightSpotDiffuseLoc = glGetUniformLocation(ourShader.Program, "spotLight.diffuse");
		GLint lightSpotSpecularLoc = glGetUniformLocation(ourShader.Program, "spotLight.specular");
		glUniform3f(lightSpotAmbientLoc, 0.2f, 0.2f, 0.2f);
		glUniform3f(lightSpotDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightSpotSpecularLoc, 1.0f, 1.0f, 1.0f);

	//	GLint cameraPosLoc = glGetUniformLocation(ourShader.Program, "cameraPos");
	//	glUniform3f(cameraPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
	//	GLint lightSpotDirLoc = glGetUniformLocation(ourShader.Program, "dirLight.direction");
	//	glUniform3f(lightSpotDirLoc, camera.Front.x, camera.Front.y, camera.Front.z);
	//	GLint lightSpotCutOffLoc = glGetUniformLocation(ourShader.Program, "light.cutOff");
	//	glUniform1f(lightSpotCutOffLoc, glm::cos(glm::radians(12.5f)));
	//	GLint lightSpotOuterCutOffLoc = glGetUniformLocation(ourShader.Program, "light.outerCutOff");
	//	glUniform1f(lightSpotOuterCutOffLoc, glm::cos(glm::radians(22.5f)));
	///*	GLint lightDirPosLoc = glGetUniformLocation(ourShader.Program, "light.direction");
	//	glUniform3f(lightDirPosLoc, -0.2f, -1.0f, -0.3f);*/
	//	GLint lightPosLoc = glGetUniformLocation(ourShader.Program, "dirLight.position");
	//	glUniform3f(lightPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

	//	GLint matDiffuseLoc = glGetUniformLocation(ourShader.Program, "material.diffuse");
	//	GLint matSpecularLoc = glGetUniformLocation(ourShader.Program, "material.specular");
	//	GLint matShininessLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
	//	/*************************************************************************************/
	//	glUniform1i(matDiffuseLoc, 0);
	//	glUniform1i(matSpecularLoc, 1);
	//	/*************************************************************************************/
	//	glUniform1f(matShininessLoc, 32.0f);

	//	GLint lightAmbientLoc = glGetUniformLocation(ourShader.Program, "dirLight.ambient");
	//	GLint lightDiffuseLoc = glGetUniformLocation(ourShader.Program, "dirLight.diffuse");
	//	GLint lightSpecularLoc = glGetUniformLocation(ourShader.Program, "dirLight.specular");
	//	glUniform3f(lightAmbientLoc, 0.2f, 0.2f, 0.2f);
	//	glUniform3f(lightDiffuseLoc, 0.5f, 0.5f, 0.5f);
	//	glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);

	//	GLint lightContantLoc = glGetUniformLocation(ourShader.Program, "light.contant");
	//	GLint lightLinearLoc = glGetUniformLocation(ourShader.Program, "light.linear");
	//	GLint lightQuadraticLoc = glGetUniformLocation(ourShader.Program, "light.quadratic");
	//	glUniform1f(lightContantLoc, 1.0f);
	//	glUniform1f(lightLinearLoc, 0.09f);
	//	glUniform1f(lightQuadraticLoc, 0.032f);
		
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projectionLoc = glGetUniformLocation(ourShader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		glBindVertexArray(VAO);
		glm::mat4 model = glm::mat4();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		for (GLuint i = 0; i < 10; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			if (i != 9)
				model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindVertexArray(0);
		mModel.Draw(ourShader);

		lightShader.Use();
		projection = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		modelLoc = glGetUniformLocation(lightShader.Program, "model");
		viewLoc = glGetUniformLocation(lightShader.Program, "view");
		projectionLoc = glGetUniformLocation(lightShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		model = glm::mat4();
		model = glm::translate(model, pointLightPos);
		model = glm::scale(model, glm::vec3(0.2f));
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

/*

// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL/SOIL.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
// Init GLFW
glfwInit();
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
glfwMakeContextCurrent(window);

// Set the required callback functions
glfwSetKeyCallback(window, key_callback);
glfwSetCursorPosCallback(window, mouse_callback);
glfwSetScrollCallback(window, scroll_callback);

// Options
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

// Initialize GLEW to setup the OpenGL Function pointers
glewExperimental = GL_TRUE;
glewInit();

// Define the viewport dimensions
glViewport(0, 0, screenWidth, screenHeight);

// Setup some OpenGL options
glEnable(GL_DEPTH_TEST);

// Setup and compile our shaders
Shader shader("C://Users/lenovo/Desktop/NeHe/NeHe/light_vs.glsl", "C://Users/lenovo/Desktop/NeHe/NeHe/light_frag.glsl");

// Load models
Model ourModel("C://Users/lenovo/Desktop/NeHe/NeHe/nanosuit/nanosuit.obj");

// Draw in wireframe
//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

// Game loop
while (!glfwWindowShouldClose(window))
{
// Set frame time
GLfloat currentFrame = glfwGetTime();
deltaTime = currentFrame - lastFrame;
lastFrame = currentFrame;

// Check and call events
glfwPollEvents();
Do_Movement();

// Clear the colorbuffer
glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

shader.Use();   // <-- Don't forget this one!
// Transformation matrices
glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
glm::mat4 view = camera.GetViewMatrix();
glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

// Draw the loaded model
glm::mat4 model;
model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
ourModel.Draw(shader);

// Swap the buffers
glfwSwapBuffers(window);
}

glfwTerminate();
return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
// Camera controls
if (keys[GLFW_KEY_W])
camera.ProcessKeyboard(FORWARD, deltaTime);
if (keys[GLFW_KEY_S])
camera.ProcessKeyboard(BACKWARD, deltaTime);
if (keys[GLFW_KEY_A])
camera.ProcessKeyboard(LEFT, deltaTime);
if (keys[GLFW_KEY_D])
camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
glfwSetWindowShouldClose(window, GL_TRUE);

if (action == GLFW_PRESS)
keys[key] = true;
else if (action == GLFW_RELEASE)
keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
if (firstMouse)
{
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
camera.ProcessMouseScroll(yoffset);
}

#pragma endregion

*/