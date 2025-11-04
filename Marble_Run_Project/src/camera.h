#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	Camera(GLFWwindow* window, glm::vec3 pos, glm::vec3 front, glm::vec3 up);

	void processInput();

	void updateCameraFront();
	void updateProjectionMatrix();
	void updateViewMatrix();

	void update(double dt = 0.0); // dt?

	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();

	glm::vec3 getCameraPos();
	glm::vec3 getCameraFront();
	glm::vec3 getCameraUp();


	//unsigned int mScrWidth = 1600;
	//unsigned int mScrHeight = 1200;
	float mCameraSpeed = 4.0f;
	float mCameraRotSpeed = 8.9f;
	float mMouseSensitivity = 0.1f;
	float mZoomSensitivity = 45.0f;

	GLFWwindow* mWindow;

	glm::vec3 mPos;
	glm::vec3 mFront;
	glm::vec3 mUp;
	float mYaw = 90.0f;
	float mPitch = 0.0f;
	double mDt = 0.0;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;
};