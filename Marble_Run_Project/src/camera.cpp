#include "camera.h"
#include <iostream>

Camera::Camera(GLFWwindow* window, glm::vec3 pos, glm::vec3 front, glm::vec3 up) :
	mWindow(window), mPos(pos), mFront(front), mUp(up)
{
    updatePitchYaw();
    updateViewMatrix();

}

void Camera::processInput()
{
    float moveAmount = static_cast<float>(mCameraSpeed * mDt);
    float zoomAmount = static_cast<float>(mZoomSensitivity * mDt);

    // Move camera
    if (mCameraMode == FREE)
    {
        if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            glm::vec3 forward = mFront;
            forward.y = 0.0f;
            mPos += glm::normalize(forward) * moveAmount;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            glm::vec3 forward = mFront;
            forward.y = 0.0f;
            mPos -= glm::normalize(forward) * moveAmount;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            mPos -= glm::normalize(glm::cross(mFront, mUp)) * moveAmount;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            mPos += glm::normalize(glm::cross(mFront, mUp)) * moveAmount;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            mPos[1] += moveAmount;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            mPos[1] -= moveAmount;
        }
    }

    // Controll follow cam
    if (mCameraMode == FOLLOW)
    {
        if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            mCameraDistance -= zoomAmount;
            if (mCameraDistance < 0.1f) mCameraDistance = 0.1f;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            mCameraDistance += zoomAmount;
        }
        if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            mCameraMode = FREE;
            updatePitchYaw();
            updateCameraFront();
        }
    }

    // Release mouse
    if (glfwGetKey(mWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
    {
        mMouseLocked = false;
        mFirstMouse = true;
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Select camera mode
    if (glfwGetKey(mWindow, GLFW_KEY_1) == GLFW_PRESS)
    {
        mCameraMode = FREE;
        updatePitchYaw();
        updateCameraFront();
    }
    if (glfwGetKey(mWindow, GLFW_KEY_2) == GLFW_PRESS)
    {
        mCameraMode = FOLLOW;
        mCameraDistance = 3.0f;
    }
}

void Camera::processMouseMovement()
{
    if (!mMouseLocked) return;

    double xpos, ypos;
    glfwGetCursorPos(mWindow, &xpos, &ypos);

    if (mFirstMouse)
    {
        mLastX = xpos;
        mLastY = ypos;
        mFirstMouse = false;
    }

    double xoffset = xpos - mLastX;
    double yoffset = mLastY - ypos;
    xoffset *= mMouseSensitivity;
    yoffset *= mMouseSensitivity;

    mLastX = xpos;
    mLastY = ypos;

    mYaw += static_cast<float>(xoffset);
    mPitch += static_cast<float>(yoffset);

    if (mPitch > 89.0f)  mPitch = 89.0f;
    if (mPitch < -89.0f) mPitch = -89.0f;

    updateCameraFront();
}

void Camera::processMouseInput()
{
    if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!mMouseLocked) {
            captureMouse();
            mMouseLocked = true;
        }
    }

}

void Camera::captureMouse()
{
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    glfwSetCursorPos(mWindow, width / 2.0, height / 2.0);

    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Camera::updateCameraFront()
{
    glm::vec3 front{};
    front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    front.y = sin(glm::radians(mPitch));
    front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mFront = glm::normalize(front);
}

void Camera::updateProjectionMatrix()
{
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);

    float fov = 45.0f;
    float aspect = static_cast<float>(width) / height;
    float near = 0.1;
    float far = 100;

    mProjectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::updateViewMatrix()
{
    if (mCameraMode == FOLLOW)
    {
        mViewMatrix = glm::lookAt(mPos, mLookAt, mUp);
    }
    else 
    {
        mViewMatrix = glm::lookAt(mPos, mPos + mFront, mUp);
    }
}

void Camera::updateOrbitCamPosition()
{
    mPos.x = mLookAt.x + mCameraDistance * sin(glm::radians(mPitch + 90.0f)) * cos(glm::radians(mYaw + 180.0f));
    mPos.y = mLookAt.y + mCameraDistance * cos(glm::radians(mPitch + 90.0f));
    mPos.z = mLookAt.z + mCameraDistance * sin(glm::radians(mPitch + 90.0f)) * sin(glm::radians(mYaw + 180.0f));
}

void Camera::updateLookAt()
{
    if (m_pBody) {
        btTransform trans;
        m_pBody->getMotionState()->getWorldTransform(trans);

        btVector3 pos = trans.getOrigin();
        mLookAt = glm::vec3(pos.getX(), pos.getY(), pos.getZ());
    }
}

void Camera::updatePitchYaw()
{
    mPitch = glm::degrees(asin(mFront.y));
    mYaw = glm::degrees(atan2(mFront.z, mFront.x));
}

void Camera::update(double dt)
{
    mDt = dt;
    processInput();
    processMouseMovement();
    processMouseInput();
    if (mCameraMode == FOLLOW) 
    {
        updateLookAt();
        updateOrbitCamPosition();
    }

    updateViewMatrix();
    updateProjectionMatrix();   
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return mProjectionMatrix;
}

glm::mat4 Camera::getViewMatrix() const
{
    return mViewMatrix;
}

glm::vec3 Camera::getCameraPos() const
{
    return mPos;
}

glm::vec3 Camera::getCameraFront() const
{
    return mFront;
}

glm::vec3 Camera::getCameraUp() const
{
    return glm::vec3(mViewMatrix[0][1], mViewMatrix[1][1], mViewMatrix[2][1]);
}

void Camera::setPBody(btRigidBody* pBody)
{
    m_pBody = pBody;
}





