#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"

struct RenderInfo;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, RenderInfo& ri);
static glm::mat4 getProjectionMatrix();
glm::mat4 getViewMatrix(RenderInfo& ri);

// settings 
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
const double CAMERA_SPEED = 2.5;
const double ROTATION_SPEED = 2;

Utils util = Utils();

//// Vertex shader source code
//const char* vertexShaderSource = R"(
//    #version 330 core
//    layout(location = 0) in vec3 in_position;
//    layout (location = 1) in vec3 in_color;
//    out vec3 ourColor;
//    uniform mat4 uModelView;
//    uniform mat4 uProjection;
//    void main() {
//        gl_Position = uProjection * uModelView * vec4(in_position, 1.0);
//        ourColor = in_color;
//    }
//)";
//
//// Fragment shader source code
//const char* fragmentShaderSource = R"(
//    #version 330 core
//    out vec4 frag_color;
//    in vec3 ourColor;
//    void main() {
//        frag_color = vec4(ourColor, 1.0f);
//    }
//)";

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

struct RenderInfo {
    Camera camera;
    Time time;
    glm::mat4 view;
    glm::mat4 model;
    glm::mat4 projection;
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

    RenderInfo renderInfo{};

    renderInfo.camera = {
        glm::vec3(0.0f, 1.0f, -10.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };

    renderInfo.model = glm::mat4(1.0f);
    renderInfo.projection = getProjectionMatrix();
    // renderInfo.model = glm::scale(renderInfo.model, glm::vec3(2.0, 2.0, 2.0));

    // Get dt
    renderInfo.time.prev = glfwGetTime();
    renderInfo.time.dt = 0;

    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    /*GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);*/

    GLuint shaderProgram = Utils::createShaderProgram("src/vertexShader.glsl", "src/fragmentShader.glsl");

    float h = -2;
    float vertices[] = {

        // Pyramid:
        // Side 1
        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,
         1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,
         0.0f, h,     0.0f,   1.0f, 0.0f, 0.0f,

        // Side 2
         1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,
         1.0f, 0.0f,  1.0f,   0.0f, 0.0f, 1.0f,
         0.0f, h,     0.0f,   1.0f, 0.0f, 0.0f,

        // Side 3
         1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
         0.0f, h,    0.0f,    1.0f, 0.0f, 0.0f,

        // Side 4 
        -1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,
         0.0f, h,     0.0f,   1.0f, 0.0f, 0.0f,

        // Bottom    
        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,
         1.0f, 0.0f,  1.0f,   0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,
         1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,
         1.0f, 0.0f,  1.0f,   0.0f, 0.0f, 1.0f,


        // Box:
        // Front face
        -1.0f, 0.5f,  1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f, 0.5f, -1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, 2.5f, -1.0f,   1.0f, 0.7f, 0.0f,
        -1.0f, 2.5f, -1.0f,   1.0f, 0.7f, 0.0f,
        -1.0f, 2.5f,  1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, 0.5f,  1.0f,   1.0f, 0.0f, 0.0f,
        
        // Back face
         1.0f, 0.5f, -1.0f,   1.0f, 0.0f, 0.0f,
         1.0f, 0.5f,  1.0f,   1.0f, 0.4f, 0.0f,
         1.0f, 2.5f,  1.0f,   1.0f, 0.7f, 0.0f,
         1.0f, 2.5f,  1.0f,   1.0f, 0.7f, 0.0f,
         1.0f, 2.5f, -1.0f,   1.0f, 0.4f, 0.0f,
         1.0f, 0.5f, -1.0f,   1.0f, 0.0f, 0.0f,
        
        // Right face
         1.0f, 0.5f,  1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, 0.5f,  1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f, 2.5f,  1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, 2.5f,  1.0f,   1.0f, 0.4f, 0.0f,
         1.0f, 2.5f,  1.0f,   1.0f, 0.7f, 0.0f,
         1.0f, 0.5f,  1.0f,   1.0f, 0.4f, 0.0f,

        // Left face
        -1.0f, 0.5f,  -1.0f,  1.0f, 0.4f, 0.0f,
         1.0f, 0.5f,  -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f, 2.5f,  -1.0f,  1.0f, 0.4f, 0.0f,
         1.0f, 2.5f,  -1.0f,  1.0f, 0.4f, 0.0f,
        -1.0f, 2.5f,  -1.0f,  1.0f, 0.7f, 0.0f,
        -1.0f, 0.5f,  -1.0f,  1.0f, 0.4f, 0.0f,

        // Top face
        -1.0f, 2.5f,  1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, 2.5f, -1.0f,   1.0f, 0.7f, 0.0f,
         1.0f, 2.5f,  -1.0f,  1.0f, 0.4f, 0.0f,
         1.0f, 2.5f,  -1.0f,  1.0f, 0.4f, 0.0f,
         1.0f, 2.5f,  1.0f,   1.0f, 0.7f, 0.0f,
        -1.0f, 2.5f,  1.0f,   1.0f, 0.4f, 0.0f,

        // Bottom face
         1.0f, 0.5f, -1.0f,   1.0f, 0.0f, 0.0f,
         1.0f, 0.5f,  1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, 0.5f,  1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f, 0.5f,  1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f, 0.5f, -1.0f,   1.0f, 0.4f, 0.0f,
         1.0f, 0.5f, -1.0f,   1.0f, 0.0f, 0.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    while (!glfwWindowShouldClose(window))
    {
        renderInfo.time.current = glfwGetTime();
        renderInfo.time.dt = renderInfo.time.current - renderInfo.time.prev;
        renderInfo.time.prev = renderInfo.time.current;

        processInput(window, renderInfo);

        glClearColor(0.2f, 0.0f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // vertex shader matrices
        renderInfo.view = getViewMatrix(renderInfo);
        glm::mat4 modelView = renderInfo.view * renderInfo.model;

        GLuint modelViewMatrixLocation = glGetUniformLocation(shaderProgram, "uModelView");
        GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "uProjection");
        glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelView));
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(renderInfo.projection));


        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / 2 );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    //glDeleteShader(vertexShader);
    //glDeleteShader(fragmentShader);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
        ri.model = glm::rotate(ri.model, rotateAmount, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        ri.model = glm::rotate(ri.model, -rotateAmount, glm::vec3(0.0f, 1.0f, 0.0f));
    }
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