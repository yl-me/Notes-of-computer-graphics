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
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);

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

	glEnable(GL_DEPTH_TEST);          //深度测试
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);      // 混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    // 混合纹理源和目标元素设置

	Shader ourShader("path/default.vert", "path/default.frag");
	Shader screenShader("path/framebuffer.vert", "path/framebuffer.frag");

	GLfloat cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		// Right face
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
	};
	
	GLfloat planeVertices[] = {
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		5.0f,  -0.5f,  5.0f,  2.0f, 0.0f,

		5.0f,  -0.5f, -5.0f,  2.0f, 2.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		5.0f,  -0.5f,  5.0f,  2.0f, 0.0f
	};

	GLfloat grassVertices[] = {
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	GLfloat quadVertices[] = {
		// Positions        // TexCoords
		-2.0f,  3.0f,  0.0f,  0.0f, 1.0f,
		-2.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		2.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-2.0f,  3.0f,  0.0f,  0.0f, 1.0f,
		2.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		2.0f,  3.0f, 0.0f,  1.0f, 1.0f
	};

	// grass position
	std::vector<glm::vec3> vegetationPos;
	vegetationPos.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	vegetationPos.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	vegetationPos.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	vegetationPos.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	vegetationPos.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	GLuint cubeVBO, cubeVAO;
	glGenBuffers(1, &cubeVBO);       // 生成一个VBO对象
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);     // 把VBO绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW); // 把顶点数据复制到缓冲的内存中
	// 顶点坐标
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)0);   // 解析顶点数据
	glEnableVertexAttribArray(0);           // 启用顶点位置属性
	// 纹理坐标
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)(3 * sizeof(float)));   // 解析顶点数据
	glEnableVertexAttribArray(1);           // 启用顶点位置属性
	glBindBuffer(GL_ARRAY_BUFFER, 0);  // 可以安全解绑，对于glVertexAttribPointer绑定的VBO对象
	glBindVertexArray(0);  // 解绑VAO


	GLuint planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// grass
	GLuint vegetationVAO, vegetationVBO;
	glGenVertexArrays(1, &vegetationVAO);
	glGenBuffers(1, &vegetationVBO);
	glBindVertexArray(vegetationVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), &grassVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// quadVAO
	GLuint quadVAO, quadVBO;  // 四边形面
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	
	GLuint FBO;      // 帧缓冲
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);       // 渲染到附加缓冲
		/*
		* When to use renderbuffer objects and when to use textures?
		* The general rule is that if you never need to sample data from a specific buffer,
		* it is wise to use a renderbuffer object for that specific buffer. If you need to
		* someday sample data from a specific buffer like colors or depth values, you should
		* use a texture attachment instead. Performance-wise it doesn't have an enormous impact though.
		*/
		// 纹理附件
		GLuint textureColorBuffer = generateAttachmentTexture(false, false);
		// 附加纹理到当前已绑定的帧缓冲对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
		
		GLuint RBO;          // 渲染缓冲附件(深度和模板)
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);   // 创建一个深度和模板渲染缓冲对象
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {    // 检查FBO
		std::cout << "ERROR::FRAMEBUFFER::FBO is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);          // 渲染到主缓冲

	GLuint cubeTexture = loadTexture("path/container.jpg");
	GLuint planeTexture = loadTexture("path/wall.jpg");
	GLuint grassTexture = loadTexture("path/blending_transparent_window.png", true);

	while (!glfwWindowShouldClose(window)) {   // 检查GLFW是否被要求退出
		glfwPollEvents();                      // 检查是否触发事件，来调用回调函数
		do_movement();

		GLfloat currentFrame = glfwGetTime();    // 当前帧的时间
		deltaTime = currentFrame - lastFrame;    // 每帧所用时间
		lastFrame = currentFrame;

//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// first pass  (附屏mirror)
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		// drawscen
		ourShader.Use();
		glm::mat4 model;
		camera.Yaw += 180.0f;
//		camera.Pitch += 180.0f;
		camera.ProcessMouseMovement(0, 0, false);
		glm::mat4 view = camera.GetViewMatrix();
		camera.Yaw -= 180.0f;
//		camera.Pitch -= 180.0f;
		camera.ProcessMouseMovement(0, 0, 0);
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projectionLoc = glGetUniformLocation(ourShader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// plane
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		model = glm::mat4();
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		

		// vegetation
		glBindVertexArray(vegetationVAO);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		std::map<float, glm::vec3> sorted;   // 排序camera到各个透明物体之间的距离
		GLuint size = vegetationPos.size();
		for (GLuint i = 0; i < size; ++i) {
			GLfloat distance = glm::length(glm::vec3(camera.Position - vegetationPos[i]));
			sorted[distance] = vegetationPos[i];
		}
		// 由远至近渲染
		for (std::map<float, glm::vec3>::reverse_iterator i = sorted.rbegin(); i != sorted.rend(); ++i) {
			model = glm::mat4();
			model = glm::translate(model, i->second);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		// order independent transparency...
		glBindVertexArray(0);

		// second pass  (主屏)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		view = camera.GetViewMatrix();
		viewLoc = glGetUniformLocation(ourShader.Program, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// plane
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		model = glm::mat4();
		modelLoc = glGetUniformLocation(ourShader.Program, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);


		// vegetation
		glBindVertexArray(vegetationVAO);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		size = vegetationPos.size();
		for (GLuint i = 0; i < size; ++i) {
			GLfloat distance = glm::length(glm::vec3(camera.Position - vegetationPos[i]));
			sorted[distance] = vegetationPos[i];
		}
		// 由远至近渲染
		for (std::map<float, glm::vec3>::reverse_iterator i = sorted.rbegin(); i != sorted.rend(); ++i) {
			model = glm::mat4();
			model = glm::translate(model, i->second);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		// order independent transparency...
		glBindVertexArray(0);

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// mirror 
		glDisable(GL_DEPTH_TEST);
		screenShader.Use();
//		glBindVertexArray(quadVAO);
//		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//		glBindVertexArray(0);

		glfwSwapBuffers(window);               // 颜色双缓冲
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteFramebuffers(1, &FBO);
	glDeleteBuffers(1, &RBO);
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

GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
	GLenum attachment_type;
	if (!depth && !stencil)
		attachment_type = GL_RGB;
	else if (depth && !stencil)
		attachment_type = GL_DEPTH_COMPONENT;
	else if (!depth && stencil)
		attachment_type = GL_STENCIL_INDEX;

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	if (!depth && !stencil)
		// 颜色缓存附件
		glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, 800, 600, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
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