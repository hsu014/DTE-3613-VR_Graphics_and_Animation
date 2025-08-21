#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"
#include "shape.h"

struct RenderInfo;
void processInput(GLFWwindow* window, RenderInfo& ri);
void initRenderInfo(RenderInfo& ri);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static glm::mat4 getProjectionMatrix();
glm::mat4 getViewMatrix(RenderInfo& ri);
void prepareShader(GLuint shaderProgram, glm::mat4 modelViewMatrix, glm::mat4 projectionMatrix);
void animate(GLFWwindow* window, RenderInfo& ri);
void draw(RenderInfo& ri);
void draw2(RenderInfo& ri);
void draw3(RenderInfo& ri);
void draw4(RenderInfo& ri);

// settings 
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
const double CAMERA_SPEED = 2.5;
const double ROTATION_SPEED = 2;

Utils util = Utils();

struct Camera {
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
};

struct Time {
    double prev;
    double current;
    double dt;
};

struct ShaderProgram {
    GLuint base;
    GLuint red;
};

struct Shapes {
    Shape* test;
    Shape* box;
    Shape* pyramid;
};

struct RenderInfo {
    Camera camera;
    Time time;
    ShaderProgram shaderProgram;
    glm::mat4 rotationMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    Shapes shape;
    
};


int main()
{
    if (!glfwInit()) {
        std::cerr << "Error initializing GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Start code", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }

    RenderInfo ri{};
    initRenderInfo(ri);
    
    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    ri.shaderProgram.base = Utils::createShaderProgram("src/vertexShader.glsl", "src/fragmentShader.glsl");
    ri.shaderProgram.red = Utils::createShaderProgram("src/vertexShader.glsl", "src/fragmentShaderRed.glsl");

    animate(window, ri);

    //Delete used resources
    glDeleteProgram(ri.shaderProgram.base);
    glDeleteProgram(ri.shaderProgram.red);

    glfwTerminate();

    return 0;
}


void processInput(GLFWwindow* window, RenderInfo& ri)
{
    float moveAmount = static_cast<float>(CAMERA_SPEED * ri.time.dt);
    float rotateAmount = static_cast<float>(ROTATION_SPEED * ri.time.dt);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Move camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        ri.camera.cameraPos += glm::normalize(ri.camera.cameraFront) * moveAmount;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        ri.camera.cameraPos -= glm::normalize(ri.camera.cameraFront) * moveAmount;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        ri.camera.cameraPos -= glm::normalize(glm::cross(ri.camera.cameraFront, ri.camera.cameraUp)) * moveAmount;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        ri.camera.cameraPos += glm::normalize(glm::cross(ri.camera.cameraFront, ri.camera.cameraUp)) * moveAmount;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        ri.camera.cameraPos[1] += moveAmount;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        ri.camera.cameraPos[1] -= moveAmount;
    }

    // Rotate model
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        ri.rotationMatrix = glm::rotate(ri.rotationMatrix, rotateAmount, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        ri.rotationMatrix = glm::rotate(ri.rotationMatrix, -rotateAmount, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}


void initRenderInfo(RenderInfo& ri)
{
    ri.camera.cameraPos = glm::vec3(0.0f, 1.0f, -10.0f);
    ri.camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    ri.camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    ri.rotationMatrix = glm::mat4(1.0f);
    ri.projectionMatrix = getProjectionMatrix();
    ri.time.prev = glfwGetTime();
    ri.time.dt = 0;

    ri.shape.test = new TestShape();
    ri.shape.box = new Box();
    ri.shape.pyramid = new Pyramid();
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


static glm::mat4 getProjectionMatrix()
{
    float fov = 45.0f;
    float aspect = static_cast<float>(SCR_WIDTH) / SCR_HEIGHT;
    float near = 0.1;
    float far = 100;

    return glm::perspective(glm::radians(fov), aspect, near, far);
}


glm::mat4 getViewMatrix(RenderInfo& ri)
{
    return glm::lookAt(ri.camera.cameraPos, ri.camera.cameraPos + ri.camera.cameraFront, ri.camera.cameraUp);
}


void prepareShader(GLuint shaderProgram, glm::mat4 modelViewMatrix, glm::mat4 projectionMatrix)
{
    glUseProgram(shaderProgram);
    GLuint modelViewMatrixLocation = glGetUniformLocation(shaderProgram, "uModelView");
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "uProjection");
    glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}


void animate(GLFWwindow* window, RenderInfo& ri)
{
    while (!glfwWindowShouldClose(window))
    {
        ri.time.current = glfwGetTime();
        ri.time.dt = ri.time.current - ri.time.prev;
        ri.time.prev = ri.time.current;

        processInput(window, ri);
        ri.viewMatrix = getViewMatrix(ri);

        glClearColor(0.2f, 0.0f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw(ri);
        draw2(ri);
        /*draw3(ri);
        draw4(ri);*/

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}


void draw(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.base, modelViewMatrix, ri.projectionMatrix);
    ri.shape.box->draw();
}


void draw2(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 1.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.base, modelViewMatrix, ri.projectionMatrix);
    ri.shape.pyramid->draw();
}


void draw3(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, -2.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.red, modelViewMatrix, ri.projectionMatrix);
    ri.shape.box->draw(); 
}


void draw4(RenderInfo& ri) 
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, -2.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.red, modelViewMatrix, ri.projectionMatrix);
    ri.shape.pyramid->draw();
}
