#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

class Camera {
public:
	enum Mode {
		FREE,
		FOLLOW
	};

	Camera(GLFWwindow* window, glm::vec3 pos, glm::vec3 front, glm::vec3 up);

	void processInput();
	void processMouseMovement();
	void processMouseInput();
	void captureMouse();

	void updateCameraFront();
	void updateProjectionMatrix();
	void updateViewMatrix();
	void updateOrbitCamPosition();
	void updateLookAt();
	void updatePitchYaw();

	void update(double dt = 0.0);

	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewMatrix() const;

	glm::vec3 getCameraPos() const;
	glm::vec3 getCameraFront() const;
	glm::vec3 getCameraUp() const;

	void setPBody(btRigidBody* pBody);

	
	// Variables
	GLFWwindow* mWindow;
	float mCameraSpeed = 4.0f;
	float mMouseSensitivity = 0.1f;
	float mZoomSensitivity = 3.0f;

	glm::vec3 mPos;
	glm::vec3 mFront;
	glm::vec3 mUp;
	glm::vec3 mLookAt = glm::vec3{0.0f, 2.0f, 0.0f};
	float mYaw = 0.0f;
	float mPitch = 0.0f;
	float mCameraDistance = 3.0f;
	double mDt = 0.0;

	double mLastX = 0.0;
	double mLastY = 0.0;
	bool mFirstMouse = true;
	bool mMouseLocked = false;

	Mode mCameraMode = FREE;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;
	btRigidBody* m_pBody = nullptr;

	
};