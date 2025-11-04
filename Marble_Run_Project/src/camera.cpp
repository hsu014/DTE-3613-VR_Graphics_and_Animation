#include "camera.h"

Camera::Camera(GLFWwindow* window, glm::vec3 pos, glm::vec3 front, glm::vec3 up) :
	mWindow(window), mPos(pos), mFront(front), mUp(up)
{
    updateViewMatrix();
}

void Camera::processInput()
{
    float moveAmount = static_cast<float>(mCameraSpeed * mDt);
    float rotateSpeed = static_cast<float>(mCameraRotSpeed * 360 * mDt);

    // Move camera
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

    // Rotate camera
    if (glfwGetKey(mWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        mYaw -= rotateSpeed * mDt;
        updateCameraFront();
    }
    if (glfwGetKey(mWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        mYaw += rotateSpeed * mDt;
        updateCameraFront();
    }
    if (glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mPitch += rotateSpeed * mDt;
        if (mPitch > 89.0f) mPitch = 89.0f;
        updateCameraFront();
    }
    if (glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mPitch -= rotateSpeed * mDt;
        if (mPitch < -89.0f) mPitch = -89.0f;
        updateCameraFront();
    }
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
    mViewMatrix = glm::lookAt(mPos, mPos + mFront, mUp);
}

void Camera::update(double dt)
{
    mDt = dt;
    updateViewMatrix();
    updateProjectionMatrix();

    processInput();
}

glm::mat4 Camera::getProjectionMatrix()
{
    return mProjectionMatrix;
}

glm::mat4 Camera::getViewMatrix()
{
    return mViewMatrix;
}

glm::vec3 Camera::getCameraPos()
{
    return mPos;
}

glm::vec3 Camera::getCameraFront()
{
    return mFront;
}

glm::vec3 Camera::getCameraUp()
{
    return mUp;
}





