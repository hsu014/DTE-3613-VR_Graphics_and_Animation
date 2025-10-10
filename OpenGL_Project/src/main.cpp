#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils.h"
#include "shape.h"
#include "particle_emitter.h"

struct RenderInfo;
struct Light;
struct MaterialType;
struct Material;
void processInput(GLFWwindow* window, RenderInfo& ri);
void initRenderInfo(RenderInfo& ri);
void createLights(RenderInfo& ri);
void createMaterials(RenderInfo& ri);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateCameraFront(RenderInfo& ri);

static glm::mat4 getProjectionMatrix();
glm::mat4 getViewMatrix(RenderInfo& ri);
void prepareShaderBasic(GLuint shaderProgram, glm::mat4 modelViewMatrix, RenderInfo& ri);
void prepareShaderPhong(GLuint shaderProgram, glm::mat4 modelMatrix, RenderInfo& ri, MaterialType& mat);
void prepareShaderParticle(GLuint shaderProgram, glm::mat4 modelViewMatrix, RenderInfo& ri);

void animate(GLFWwindow* window, RenderInfo& ri);
void draw(RenderInfo& ri);
void draw2(RenderInfo& ri);
void draw3(RenderInfo& ri);
void draw4(RenderInfo& ri);
void drawPlane(RenderInfo& ri);
void drawSphere(RenderInfo& ri);
void drawLightSpheres(RenderInfo& ri);
void drawEmitter(RenderInfo& ri);

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
    GLuint pair;
    GLuint texture;
    GLuint phong;
    GLuint particle;
};

struct AmbientLight {
    glm::vec4 color;
};

struct DirectionalLight {
    glm::vec3 direction;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

struct PointLight {
    glm::vec3 position;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    
    float constant;
    float linear;
    float quadratic;
};

struct Light {
    AmbientLight ambient;
    std::vector<DirectionalLight> directional;
    std::vector<PointLight> point;
};

struct MaterialType {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
};

struct Material {
    MaterialType blue;
    MaterialType gold;
    MaterialType silver;
};

struct RenderInfo {
    Camera camera;
    Light light;
    Material material;
    Time time;
    ShaderProgram shaderProgram;
    glm::mat4 rotationMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    std::map<std::string, Shape*> shape;
    std::map<std::string, GLuint> texture;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    Emitter emitter;  
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
    ri.shaderProgram.pair = Utils::createShaderProgram("src/vertexShaderPair.glsl", "src/fragmentShaderPair.glsl");
    ri.shaderProgram.texture = Utils::createShaderProgram("src/vertexShader.glsl", "src/fragmentShaderTexture.glsl");
    ri.shaderProgram.phong = Utils::createShaderProgram("src/vertexShaderPhong.glsl", "src/fragmentShaderPhong.glsl");
    ri.shaderProgram.particle = Utils::createShaderProgram("src/vertexShaderParticle.glsl", "src/fragmentShaderParticle.glsl");

    animate(window, ri);

    //Delete used resources
    glDeleteProgram(ri.shaderProgram.base);
    glDeleteProgram(ri.shaderProgram.red);
    glDeleteProgram(ri.shaderProgram.texture);
    glDeleteProgram(ri.shaderProgram.phong);
    glDeleteProgram(ri.shaderProgram.particle);

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
    updateCameraFront(ri);

    ri.rotationMatrix = glm::mat4(1.0f);
    ri.projectionMatrix = getProjectionMatrix();
    ri.time.prev = glfwGetTime();
    ri.time.dt = 0;

    createLights(ri);

    createMaterials(ri);

    // Textures
    ri.texture["heightmap_1"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_1.png");
    ri.texture["heightmap_2"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_2.png");
    ri.texture["heightmap_3"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_3.png");
    ri.texture["heightmap_4"] = Utils::loadTexture("src/Textures/Heightmaps/heightmap_4.png");
    ri.texture["chicken"] = Utils::loadTexture("src/Textures/mc_chicken.jpeg");
    ri.texture["particle"] = Utils::loadTexture("src/Textures/particle.png");
    ri.texture["fire"] = Utils::loadTexture("src/Textures/fire.png");

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


    // Create shapes
    ri.shape["test"] = new TestShape();
    ri.shape["box"] = new Box();
    ri.shape["pyramid"] = new Pyramid();
    ri.shape["sphere"] = new Sphere(20, 20);

    // Create heightmap plane:
    std::string mapName = "heightmap_4";
    ri.shape["plane"] = new CompositePlane(
        ri.texture[mapName], ri.heightMap[mapName]);

    // Particle emitter
    ri.emitter = Emitter(2000, 2.0f, 0.5f, ri.texture["particle"]);
}


void createLights(RenderInfo& ri)
{
    // Ambient
    ri.light.ambient.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Directional
    DirectionalLight dirLight{};
    dirLight.direction = glm::vec3(0.0f, -5.0f, 0.0f);

    dirLight.ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
    dirLight.diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.specular = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    ri.light.directional.push_back(dirLight);

    // Point
    PointLight pointLight{};
    pointLight.position = glm::vec3(-1.0f, 2.0f, 0.0f);

    pointLight.ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
    pointLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    ri.light.point.push_back(pointLight);

    pointLight.position = glm::vec3(-1.0f, 0.0f, 2.0f);
    ri.light.point.push_back(pointLight);

}


void createMaterials(RenderInfo& ri)
{
    ri.material.blue = {
    glm::vec4(0.0f, 0.0f, 1.0f, 1),
    glm::vec4(0.0f, 0.0f, 1.0f, 1),
    glm::vec4(1.0f, 1.0f, 1.0f, 1),
    40.0f
    };

    ri.material.gold = {
    glm::vec4(0.2473f, 0.1995f, 0.0745f, 1),
    glm::vec4(0.7516f, 0.6065f, 0.2265f, 1),
    glm::vec4(0.6283f, 0.5559f, 0.3661f, 1),
    51.2f
    };

    ri.material.silver = {
    glm::vec4(0.1923f, 0.1923f, 0.1923f, 1),
    glm::vec4(0.5075f, 0.5075f, 0.5075f, 1),
    glm::vec4(0.5083f, 0.5083f, 0.5083f, 1),
    51.2f
    };
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


void prepareShaderBasic(GLuint shaderProgram, glm::mat4 modelViewMatrix, RenderInfo& ri)
{
    glUseProgram(shaderProgram);
    shaderSetMat4(shaderProgram, "uModelView", modelViewMatrix);
    shaderSetMat4(shaderProgram, "uProjection", ri.projectionMatrix);
}

void prepareShaderPhong(GLuint shaderProgram, glm::mat4 modelMatrix, RenderInfo& ri, MaterialType& mat)
{
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;
    
    // Bind lighting and material info to Phong shader
    glUseProgram(shaderProgram);
    shaderSetMat4(shaderProgram, "uModel", modelMatrix);
    shaderSetMat4(shaderProgram, "uModelView", modelViewMatrix);
    shaderSetMat4(shaderProgram, "uProjection", ri.projectionMatrix);
    shaderSetMat4(shaderProgram, "uNormal", normalMatrix);
    shaderSetVec3(shaderProgram, "viewPos", ri.camera.cameraPos);
    shaderSetInt(shaderProgram, "numPointLights", ri.light.point.size());

    // Ambient
    shaderSetVec4(shaderProgram, "ambientLight", ri.light.ambient.color);

    // Directional
    DirectionalLight dirLight = ri.light.directional[0];
    shaderSetVec3(shaderProgram, "dirLight.direction", dirLight.direction);
    shaderSetVec4(shaderProgram, "dirLight.ambient", dirLight.ambient);
    shaderSetVec4(shaderProgram, "dirLight.diffuse", dirLight.diffuse);
    shaderSetVec4(shaderProgram, "dirLight.specular", dirLight.specular);

    // Point
    for (int i = 0; i < ri.light.point.size(); i++) {
        PointLight pointLight = ri.light.point[i];

        shaderSetVec3(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].position").c_str(), pointLight.position);
        shaderSetVec4(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].ambient").c_str(), pointLight.ambient);
        shaderSetVec4(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].diffuse").c_str(), pointLight.diffuse);
        shaderSetVec4(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].specular").c_str(), pointLight.specular);
        shaderSetFloat(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].constant").c_str(), pointLight.constant);
        shaderSetFloat(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].linear").c_str(), pointLight.linear);
        shaderSetFloat(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].quadratic").c_str(), pointLight.quadratic);
    }

    // Material
    shaderSetVec4(shaderProgram, "material.ambient", mat.ambient);
    shaderSetVec4(shaderProgram, "material.diffuse", mat.diffuse);
    shaderSetVec4(shaderProgram, "material.specular", mat.specular);
    shaderSetFloat(shaderProgram, "material.shininess", mat.shininess);

}

void prepareShaderParticle(GLuint shaderProgram, glm::mat4 modelViewMatrix, RenderInfo& ri)
{
    glm::vec3 up = glm::vec3(ri.viewMatrix[0][1], ri.viewMatrix[1][1], ri.viewMatrix[2][1]);

    glUseProgram(shaderProgram);
    shaderSetMat4(shaderProgram, "uModelView", modelViewMatrix);
    shaderSetMat4(shaderProgram, "uProjection", ri.projectionMatrix);
    shaderSetVec3(shaderProgram, "cameraUp", up);
    shaderSetVec3(shaderProgram, "cameraFront", ri.camera.cameraFront);
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
        drawEmitter(ri);
        
        ////draw3(ri);
        draw4(ri);

        drawPlane(ri);

        drawSphere(ri);
        drawLightSpheres(ri);

        

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
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.0f, 5.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShaderBasic(ri.shaderProgram.base, modelViewMatrix, ri);
    //prepareShaderPhong(ri.shaderProgram.phong, modelMatrix, ri, ri.material.gold);
    ri.shape["box"]->draw();
}


void draw2(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 1.0f, 5.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShaderBasic(ri.shaderProgram.base, modelViewMatrix, ri);
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

    prepareShaderBasic(ri.shaderProgram.red, modelViewMatrix, ri);
    ri.shape["box"]->draw();
}


void draw4(RenderInfo& ri) 
{
    // Task 4

    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.0f, 1.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShaderBasic(ri.shaderProgram.pair, modelViewMatrix, ri);
    float changeColor = cos(ri.time.current);
    shaderSetFloat(ri.shaderProgram.pair, "changeColor", changeColor);

    ri.shape["pyramid"]->draw();
}


void drawPlane(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -2.5f, 0.0f));

    // Rotate
    // modelMatrix *= ri.rotationMatrix;
    // modelMatrix = glm::rotate(modelMatrix, glm::half_pi<float>(), glm::vec3(-1.0f, 0.0f, 0.0f));

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f) * 10.0f);

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShaderBasic(ri.shaderProgram.texture, modelViewMatrix, ri);
    ri.shape["plane"]->draw();
}


void drawSphere(RenderInfo& ri)
{
    //M=I*T*O*R*S, der O=R*T
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(4.0f, 1.0f, 0.0f));

    // Rotate
    modelMatrix *= ri.rotationMatrix;

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShaderPhong(ri.shaderProgram.phong, modelMatrix, ri, ri.material.blue);
    //prepareShaderBasic(ri.shaderProgram.texture, modelViewMatrix, ri);

    ri.shape["sphere"]->draw();
}


void drawLightSpheres(RenderInfo& ri)
{
    for (const PointLight& light : ri.light.point) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::translate(modelMatrix, light.position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));

        glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

        prepareShaderBasic(ri.shaderProgram.base, modelViewMatrix, ri);
        ri.shape["sphere"]->draw();
    }
}


void drawEmitter(RenderInfo& ri)
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 1.0f, 5.0f));

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f) * 1.0f);

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    prepareShaderParticle(ri.shaderProgram.particle, modelViewMatrix, ri);

    ri.emitter.updateParticles(ri.time.dt);
    ri.emitter.renderParticles(ri.shaderProgram.particle);
}