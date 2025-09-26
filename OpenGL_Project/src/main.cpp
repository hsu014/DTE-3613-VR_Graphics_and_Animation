#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"
#include "shape.h"

struct RenderInfo;
void processInput(GLFWwindow* window, RenderInfo& ri);
void initRenderInfo(RenderInfo& ri);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateCameraFront(RenderInfo& ri);

static glm::mat4 getProjectionMatrix();
glm::mat4 getViewMatrix(RenderInfo& ri);
void prepareShader(GLuint shaderProgram, glm::mat4 modelViewMatrix, glm::mat4 projectionMatrix);
void animate(GLFWwindow* window, RenderInfo& ri);
void draw(RenderInfo& ri);
void draw2(RenderInfo& ri);
void draw3(RenderInfo& ri);
void draw4(RenderInfo& ri);
void drawPlane(RenderInfo& ri);
void drawSphere(RenderInfo& ri);

//float PI = glm::pi<float>();
// 
// settings 
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
//const unsigned int SCR_WIDTH = 1100;
//const unsigned int SCR_HEIGHT = 800;

unsigned int CUR_WIDTH = SCR_WIDTH;
unsigned int CUR_HEIGHT = SCR_HEIGHT;

const double CAMERA_SPEED = 2.5;
const double ROTATION_SPEED = 2;


Utils util = Utils();

struct Camera {
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float yaw;
    float pitch;
};

struct Time {
    double prev;
    double current;
    double dt;
};

struct ShaderProgram {
    GLuint base;
    GLuint red;
    GLuint texture;
};

struct RenderInfo {
    Camera camera;
    Time time;
    ShaderProgram shaderProgram;
    glm::mat4 rotationMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    std::map<std::string, Shape*> shape;
    std::map<std::string, GLuint> texture;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    
};

/* Light
Ambient
    color: r, g ,b

Directional
    direction vec3
    color: r, g ,b

PointLight
    position vec3
    intensity
    color: r, g ,b

SpotLight?

*/


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
    ri.shaderProgram.texture = Utils::createShaderProgram("src/vertexShader.glsl", "src/fragmentShaderTexture.glsl");

    animate(window, ri);

    //Delete used resources
    glDeleteProgram(ri.shaderProgram.base);
    glDeleteProgram(ri.shaderProgram.red);
    glDeleteProgram(ri.shaderProgram.texture);

    glfwTerminate();

    return 0;
}


void processInput(GLFWwindow* window, RenderInfo& ri)
{
    float moveAmount = static_cast<float>(CAMERA_SPEED * ri.time.dt);
    float rotateSpeed = 50.0f; // degrees per second
    float rotateAmount = static_cast<float>(ROTATION_SPEED * ri.time.dt);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Move camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        glm::vec3 forward = ri.camera.cameraFront;
        forward.y = 0.0f;
        ri.camera.cameraPos += glm::normalize(forward) * moveAmount;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm::vec3 forward = ri.camera.cameraFront;
        forward.y = 0.0f;
        ri.camera.cameraPos -= glm::normalize(forward) * moveAmount;
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

    // Rotate camera
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        ri.camera.yaw -= rotateSpeed * ri.time.dt;
        updateCameraFront(ri);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        ri.camera.yaw += rotateSpeed * ri.time.dt;
        updateCameraFront(ri);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        ri.camera.pitch += rotateSpeed * ri.time.dt;
        if (ri.camera.pitch > 89.0f) ri.camera.pitch = 89.0f;
        updateCameraFront(ri);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        ri.camera.pitch -= rotateSpeed * ri.time.dt;
        if (ri.camera.pitch < -89.0f) ri.camera.pitch = -89.0f;
        updateCameraFront(ri);
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
    ri.camera.yaw = 90.0f;
    ri.camera.pitch = 0.0f;
    ri.rotationMatrix = glm::mat4(1.0f);
    ri.projectionMatrix = getProjectionMatrix();
    ri.time.prev = glfwGetTime();
    ri.time.dt = 0;

    updateCameraFront(ri);

    // Textures
    ri.texture["heightmap_1"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_1.png");
    ri.texture["heightmap_2"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_2.png");
    ri.texture["heightmap_3"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_3.png");
    ri.texture["heightmap_4"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_4.png");
    ri.texture["chicken"] = Utils::loadTexture("src/Textures/mc_chicken.jpeg");

    // Heightmap
    ri.heightMap["heightmap_1"] = 
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/Textures/Heightmaps/heightmap_1.png"));
    ri.heightMap["heightmap_2"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/Textures/Heightmaps/heightmap_2.png"));
    ri.heightMap["heightmap_3"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/Textures/Heightmaps/heightmap_3.png"));
    ri.heightMap["heightmap_4"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/Textures/Heightmaps/heightmap_4.png"));
    ri.heightMap["chicken"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/Textures/Heightmaps/mc_chicken.jpeg"));


    // Shapes
    ri.shape["test"] = new TestShape();
    ri.shape["box"] = new Box();
    ri.shape["pyramid"] = new Pyramid();
    ri.shape["sphere"] = new Sphere(20, 20);

    // Create heightmap plane:
    std::string mapName = "heightmap_4";
    ri.shape["plane"] = new CompositePlane(
        ri.texture[mapName], ri.heightMap[mapName]);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    
    glViewport(0, 0, width, height);
    CUR_WIDTH = width;
    CUR_HEIGHT = height;
}

void updateCameraFront(RenderInfo& ri)
{
    glm::vec3 front;
    front.x = cos(glm::radians(ri.camera.yaw)) * cos(glm::radians(ri.camera.pitch));
    front.y = sin(glm::radians(ri.camera.pitch));
    front.z = sin(glm::radians(ri.camera.yaw)) * cos(glm::radians(ri.camera.pitch));
    ri.camera.cameraFront = glm::normalize(front);
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

        //draw(ri);
        //draw2(ri);
        //draw3(ri);
        //draw4(ri);

        drawPlane(ri);
        drawSphere(ri);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}


void draw(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    //modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.base, modelViewMatrix, ri.projectionMatrix);
    ri.shape["box"]->draw();
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
    ri.shape["pyramid"]->draw();
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
    ri.shape["box"]->draw();
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
    ri.shape["pyramid"]->draw();
}


void drawPlane(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.5f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;
    // modelMatrix = glm::rotate(modelMatrix, glm::half_pi<float>(), glm::vec3(-1.0f, 0.0f, 0.0f));

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.texture, modelViewMatrix, ri.projectionMatrix);
    ri.shape["plane"]->draw();
}

void drawSphere(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShader(ri.shaderProgram.base, modelViewMatrix, ri.projectionMatrix);
    ri.shape["sphere"]->draw();
}
