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
	glm::mat4x4 myLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
	{
		glm::vec3 zaixs = glm::normalize(eye - center);
		glm::vec3 xaixs = glm::normalize(glm::cross(glm::normalize(up), zaixs));
		glm::vec3 yaixs = glm::cross(zaixs, xaixs);

		glm::mat4x4 result;
		result[0][0] = xaixs.x;
		result[0][1] = yaixs.x;
		result[0][2] = zaixs.x;
		result[0][3] = 0;
		result[1][0] = xaixs.y;
		result[1][1] = yaixs.y;
		result[1][2] = zaixs.y;
		result[1][3] = 0;
		result[2][0] = xaixs.z;
		result[2][1] = yaixs.z;
		result[2][2] = zaixs.z;
		result[2][3] = 0;
		result[3][0] = -eye.x * xaixs.x - eye.y * xaixs.y - eye.z * xaixs.z;
		result[3][1] = -eye.x * yaixs.x - eye.y * yaixs.y - eye.z * yaixs.z;
		result[3][2] = -eye.x * zaixs.x - eye.y * zaixs.y - eye.z * zaixs.z;
		result[3][3] = 1;

		return result;
	}

	// 返回视图矩阵(使用欧拉角和lookAt矩阵)
	glm::mat4 GetViewMatrix()
	{
		return myLookAt(this->Position, this->Position + this->Front, this->Up);
	}

	

	// 相机移动，变化的速率
	void Processkeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;
		if (direction == FORWARD) {
			this->Position += this->Front * velocity;
		}
		if (direction == BACKWARD) {
			this->Position -= this->Front * velocity;
		}
		if (direction == RIGHT) {
			this->Position += this->Right * velocity;
		}
		if (direction == LEFT) {
			this->Position -= this->Right * velocity;
		}
	//	this->Position.y = 0;                      // practice
	}

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
	void ProcessMouseScroll(GLfloat yoffset)
	{
		if (this->Zoom >= 1.0f && this->Zoom <= 90.0f) {
			this->Zoom -= yoffset;
		}
		if (this->Zoom <= 1.0f) {
			this->Zoom = 1.0f;
		}
		if (this->Zoom >= 90.0f) {
			this->Zoom = 90.0f;
		}
	}

private: 
	// 欧拉角
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		front.y = sin(glm::radians(this->Pitch));
		front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		this->Front = glm::normalize(front);
		
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
	}
};