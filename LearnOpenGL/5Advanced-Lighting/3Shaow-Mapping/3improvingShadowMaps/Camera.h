#pragma once

#include <vector>

#include <gl\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

// 移动方向
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// 默认值
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVITY = 0.25f;
const GLfloat ZOOM = 45.0f;

class Camera {
public:
	// 属性
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// 欧拉角
	GLfloat Yaw;
	GLfloat Pitch;
	
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;

	// 向量构造
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
		MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// 值构造
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw,
		GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
		MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		this->Position = glm::vec3(posX, posY, posZ);
		this->WorldUp = glm::vec3(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// 矩阵相乘后转置
	glm::mat4x4 myLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up);

	// 返回视图矩阵(使用欧拉角和lookAt矩阵)
	glm::mat4 GetViewMatrix();
	

	// 相机移动，变化的速率
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);

	// 鼠标
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Pitch += yoffset;
		this->Yaw += xoffset;

		if (constrainPitch) {
			if (this->Pitch < -89.0f) {
				this->Pitch = -89.0f;
			}
			if (this->Pitch > 89.0f) {
				this->Pitch = 89.0f;
			}
		}
		this->updateCameraVectors();
	}

	// 滚轮
	void ProcessMouseScroll(GLfloat yoffset);

private: 
	// 欧拉角
	void updateCameraVectors();
};