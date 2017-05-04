#include "Camera.h"

// 矩阵相乘后转置
glm::mat4x4 Camera::myLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
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
glm::mat4 Camera::GetViewMatrix()
{
	return myLookAt(this->Position, this->Position + this->Front, this->Up);
}

// 相机移动，变化的速率
void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
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



// 滚轮
void Camera::ProcessMouseScroll(GLfloat yoffset)
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

// 欧拉角
void Camera::updateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	front.y = sin(glm::radians(this->Pitch));
	front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	this->Front = glm::normalize(front);

	this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
	this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}