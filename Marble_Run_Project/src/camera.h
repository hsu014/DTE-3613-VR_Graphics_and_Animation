#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	Camera(GLFWwindow* window, glm::vec3 pos, glm::vec3 front, glm::vec3 up);

	void processInput();
	void processMouseMovement();
	void processMouseInput();
	void captureMouse();

	void updateCameraFront();
	void updateProjectionMatrix();
	void updateViewMatrix();

	void update(double dt = 0.0);

	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewMatrix() const;

	glm::vec3 getCameraPos() const;
	glm::vec3 getCameraFront() const;
	glm::vec3 getCameraUp() const;

	
	float mCameraSpeed = 4.0f;
	float mCameraRotSpeed = 8.9f;
	float mMouseSensitivity = 0.1f;
	float mZoomSensitivity = 45.0f;

	GLFWwindow* mWindow;

	glm::vec3 mPos;
	glm::vec3 mFront;
	glm::vec3 mUp;
	float mYaw = 0.0f;
	float mPitch = 0.0f;
	double mDt = 0.0;

	double mLastX = 0.0;
	double mLastY = 0.0;
	bool mFirstMouse = true;
	bool mMouseLocked = false;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;
};