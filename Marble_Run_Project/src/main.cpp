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
#include "render_info.h"

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

void processInput(GLFWwindow* window, RenderInfo& ri);
void initRenderInfo(RenderInfo& ri);
void loadTextures(RenderInfo& ri);
void loadSkyboxTextures(RenderInfo& ri);
void loadHeightmaps(RenderInfo& ri);
void createLights(RenderInfo& ri);
void createMaterials(RenderInfo& ri);
void createShapes(RenderInfo& ri);
void testBulletShapes(RenderInfo& ri);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateCameraFront(RenderInfo& ri);

static glm::mat4 getProjectionMatrix();
glm::mat4 getViewMatrix(RenderInfo& ri);

void animate(GLFWwindow* window, RenderInfo& ri);
void drawScene(RenderInfo& ri);

void drawEmitter(RenderInfo& ri); // Move to scene later


// settings 
unsigned int SCR_WIDTH = 1600;
unsigned int SCR_HEIGHT = 1200;
// unsigned int SCR_WIDTH = 1100;
// unsigned int SCR_HEIGHT = 800;

const double CAMERA_SPEED = 4;
const double CAMERA_ROT_SPEED = 8;
const double ROTATION_SPEED = 2;

Utils util = Utils();


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
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Compile and link shaders
    ri.shaderProgram.base = Utils::createShaderProgram("src/shader/vertexShader.glsl", "src/shader/fragmentShader.glsl");
    ri.shaderProgram.phong = Utils::createShaderProgram("src/shader/vertexShaderPhong.glsl", "src/shader/fragmentShaderPhong.glsl");
    ri.shaderProgram.skybox = Utils::createShaderProgram("src/shader/vertexShaderSkybox.glsl", "src/shader/fragmentShaderSkybox.glsl");
    ri.shaderProgram.particle = Utils::createShaderProgram("src/shader/vertexShaderParticle.glsl", "src/shader/fragmentShaderParticle.glsl");
    
    ri.scene.setShaders(ri.shaderProgram.base, ri.shaderProgram.phong, ri.shaderProgram.skybox);

    // Init bullet
    ri.bullet.pCollisionConfiguration = new btDefaultCollisionConfiguration();
    ri.bullet.pDispatcher = new btCollisionDispatcher(ri.bullet.pCollisionConfiguration);
    ri.bullet.pBroadphase = new btDbvtBroadphase();
    ri.bullet.pSolver = new btSequentialImpulseConstraintSolver();
    ri.bullet.pWorld = new btDiscreteDynamicsWorld(
        ri.bullet.pDispatcher, ri.bullet.pBroadphase, ri.bullet.pSolver, ri.bullet.pCollisionConfiguration);
    ri.bullet.pWorld->setGravity(btVector3(0, -9.81f, 0));

    // Create shapes
    createShapes(ri);
    testBulletShapes(ri);

    animate(window, ri);

    // Delete used resources
    glDeleteProgram(ri.shaderProgram.base);
    glDeleteProgram(ri.shaderProgram.phong);
    glDeleteProgram(ri.shaderProgram.skybox);
    glDeleteProgram(ri.shaderProgram.particle);

    // Shutdown bullet
    delete ri.bullet.pWorld;
    delete ri.bullet.pSolver;
    delete ri.bullet.pBroadphase;
    delete ri.bullet.pDispatcher;
    delete ri.bullet.pCollisionConfiguration;

    glfwTerminate();

    return 0;
}


void processInput(GLFWwindow* window, RenderInfo& ri)
{
    float moveAmount = static_cast<float>(CAMERA_SPEED * ri.time.dt);
    float rotateSpeed = static_cast<float>(CAMERA_ROT_SPEED * 360 * ri.time.dt); // 50.0f; // degrees per second
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
}


void initRenderInfo(RenderInfo& ri)
{
    ri.camera.cameraPos = glm::vec3(0.0f, 1.0f, -10.0f);
    ri.camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    ri.camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    ri.camera.yaw = 90.0f;
    ri.camera.pitch = 0.0f;
    updateCameraFront(ri);

    ri.projectionMatrix = getProjectionMatrix();
    ri.time.prev = glfwGetTime();
    ri.time.dt = 0;

    ri.scene = Scene();

    createLights(ri);
    createMaterials(ri);

    loadTextures(ri);
    loadSkyboxTextures(ri);
    loadHeightmaps(ri);

    Skybox* skybox = new Skybox(ri.skyboxTexture["sky_42"]);
    ri.scene.addSkybox(skybox);
}

void loadTextures(RenderInfo& ri)
{
    ri.texture["heightmap_1"] = Utils::loadTexture("src/textures/heightmaps/heightmap_1.png");
    ri.texture["heightmap_2"] = Utils::loadTexture("src/textures/heightmaps/heightmap_2.png");
    ri.texture["heightmap_3"] = Utils::loadTexture("src/textures/heightmaps/heightmap_3.png");
    ri.texture["heightmap_4"] = Utils::loadTexture("src/textures/heightmaps/heightmap_4.png");
    ri.texture["chicken"] = Utils::loadTexture("src/textures/mc_chicken.jpeg");
    ri.texture["particle"] = Utils::loadTexture("src/textures/particle.png");
    ri.texture["fire"] = Utils::loadTexture("src/textures/fire.png");
    ri.texture["wood"] = Utils::loadTexture("src/textures/wood.png");
    ri.texture["gray_brick"] = Utils::loadTexture("src/textures/gray_brick.jpg");
    ri.texture["grass"] = Utils::loadTexture("src/textures/grass.png");
}

void loadSkyboxTextures(RenderInfo& ri)
{
    ri.skyboxTexture["sky_22"] = Utils::loadCubeMap("src/textures/skybox/sky_22");
    ri.skyboxTexture["sky_27"] = Utils::loadCubeMap("src/textures/skybox/sky_27");
    ri.skyboxTexture["sky_42"] = Utils::loadCubeMap("src/textures/skybox/sky_42");
}

void loadHeightmaps(RenderInfo& ri)
{
    ri.heightMap["heightmap_1"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/textures/heightmaps/heightmap_1.png"));
    ri.heightMap["heightmap_2"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/textures/heightmaps/heightmap_2.png"));
    ri.heightMap["heightmap_3"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/textures/heightmaps/heightmap_3.png"));
    ri.heightMap["heightmap_4"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/textures/heightmaps/heightmap_4.png"));
    ri.heightMap["chicken"] =
        std::make_shared<std::vector<std::vector<float>>>(Utils::loadHeightMap("src/textures/heightmaps/mc_chicken.jpeg"));

}

void createLights(RenderInfo& ri)
{
    // Ambient
    ri.scene.setAmbientLight(glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

    // Directional
    DirectionalLight dirLight{};
    dirLight.direction = glm::vec3(0.0f, -5.0f, 3.0f);

    dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    dirLight.diffuse = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    // dirLight.specular = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    
    ri.scene.addDirectionLight(dirLight);

    // Point
    PointLight pointLight{};
    pointLight.position = glm::vec3(-1.0f, 2.0f, 0.0f);

    pointLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    pointLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    ri.scene.addPointLight(pointLight, true);

    pointLight.position = glm::vec3(-1.0f, 0.1f, 2.0f);
    ri.scene.addPointLight(pointLight, true);

}

void createMaterials(RenderInfo& ri)
{
    ri.material["white"] = {
    glm::vec4(1.0f, 1.0f, 1.0f, 1),
    glm::vec4(1.0f, 1.0f, 1.0f, 1),
    glm::vec4(1.0f, 1.0f, 1.0f, 1),
    40.0f
    };

    ri.material["blue"] = {
    glm::vec4(0.0f, 0.0f, 1.0f, 1),
    glm::vec4(0.0f, 0.0f, 1.0f, 1),
    glm::vec4(1.0f, 1.0f, 1.0f, 1),
    40.0f
    };

    ri.material["gold"] = {
    glm::vec4(0.2473f, 0.1995f, 0.0745f, 1),
    glm::vec4(0.7516f, 0.6065f, 0.2265f, 1),
    glm::vec4(0.6283f, 0.5559f, 0.3661f, 1),
    51.2f
    };

    ri.material["silver"] = {
    glm::vec4(0.1923f, 0.1923f, 0.1923f, 1),
    glm::vec4(0.5075f, 0.5075f, 0.5075f, 1),
    glm::vec4(0.5083f, 0.5083f, 0.5083f, 1),
    51.2f
    };
}


void createShapes(RenderInfo& ri)
{
    // Create shapes and put into scene
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Basic shape:
    Shape* box = new Box(0.2, 0.2, 0.1);
    box->useTexture(ri.texture["fire"]);
    ri.scene.addBaseShape(box);

    Shape* box2 = new Box(2.0, 2.0, 2.0);
    box2->useTexture(ri.texture["gray_brick"]);
    box2->setMaterial(ri.material["silver"]);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.5f, 5.0f));
    box2->setModelMatrix(modelMatrix);
    ri.scene.addPhongShape(box2);

    // Phong shape
    Shape* pyramid = new CompositePlane(1.0, 1.5, 1.0);
    pyramid->setMaterial(ri.material["gold"]);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.0f, 1.0f, 0.0f));
    pyramid->setModelMatrix(modelMatrix);
    ri.scene.addPhongShape(pyramid);


    Shape* sphere = new Sphere(1.0, 20, 20);
    sphere->setMaterial(ri.material["gold"]);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(4.0f, 1.0f, 0.0f));
    sphere->setModelMatrix(modelMatrix);
    ri.scene.addPhongShape(sphere);

    Shape* sphere2 = new Sphere(0.5, 20, 20);
    sphere2->setMaterial(ri.material["silver"]);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(4.0f, 3.0f, 0.0f));
    sphere2->setModelMatrix(modelMatrix);
    ri.scene.addPhongShape(sphere2);

}

void testBulletShapes(RenderInfo& ri)
{
    // Plane normal pointing up (y-axis), and plane constant (distance from origin)
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

    // Default motion state: plane at world origin
    btDefaultMotionState* groundMotionState =
        new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

    // Static body -> mass = 0
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
        0,                     // mass
        groundMotionState,
        groundShape,
        btVector3(0, 0, 0));   // inertia (not used for static objects)

    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setRestitution(0.6f);  // bounciness
    groundRigidBody->setFriction(0.5f);     // rolling resistance
    

    // Optional: make sure it doesn’t get deactivated
    //groundRigidBody->setActivationState(DISABLE_DEACTIVATION);

    // Finally, add it to the world
    ri.bullet.pWorld->addRigidBody(groundRigidBody);

    Shape* plane = new Plane(20, 20);
    plane->useTexture(ri.texture["grass"]);
    //plane->setMaterial(ri.material["gold"]);
    ri.scene.addPhongShape(plane);




    // Test sphere
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    btScalar mass = 1.0f;
    btScalar radius = 0.2f;

    // 1. Create the collision shape
    btCollisionShape* sphereShape = new btSphereShape(radius);

    // 2. Calculate the inertia for the given mass
    btVector3 sphereInertia(0, 0, 0);
    sphereShape->calculateLocalInertia(mass, sphereInertia);

    // 3. Set the initial transform (position & rotation)
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0, 2, 0));

    // 4. Create the motion state (keeps transform in sync)
    btDefaultMotionState* sphereMotionState = new btDefaultMotionState(startTransform);

    // 5. Construct the rigid body
    btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(
        mass,
        sphereMotionState,
        sphereShape,
        sphereInertia);

    btRigidBody* sphereRigidBody = new btRigidBody(sphereRigidBodyCI);
    sphereRigidBody->setRestitution(0.6f);  // bounciness
    sphereRigidBody->setFriction(0.8f);     // rolling resistance
    sphereRigidBody->setActivationState(DISABLE_DEACTIVATION);
    //sphereRigidBody->setLinearVelocity(btVector3(0.3f, 0.0f, 0.0f));
    sphereRigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, -3.0f));

    ri.bullet.pWorld->addRigidBody(sphereRigidBody);

    // Phong shape
    Shape* sphere = new Sphere(radius, 40, 40);
    //sphere->setMaterial(ri.material["gold"]);
    sphere->useTexture(ri.texture["wood"]);
    sphere->setPBody(sphereRigidBody);
    ri.scene.addPhongShape(sphere);

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void updateCameraFront(RenderInfo& ri)
{
    glm::vec3 front{};
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

void animate(GLFWwindow* window, RenderInfo& ri)
{
    while (!glfwWindowShouldClose(window))
    {
        ri.time.current = glfwGetTime();
        ri.time.dt = ri.time.current - ri.time.prev;
        ri.time.prev = ri.time.current;

        processInput(window, ri);
        ri.viewMatrix = getViewMatrix(ri);
        ri.projectionMatrix = getProjectionMatrix();

        glClearColor(0.2f, 0.0f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawScene(ri);

        ri.bullet.pWorld->stepSimulation(float(ri.time.dt));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}

void drawScene(RenderInfo& ri)
{
    // Draw shapes in scene
    ri.scene.update(ri.viewMatrix, ri.projectionMatrix, ri.camera.cameraPos);
    ri.scene.draw();

}

void drawEmitter(RenderInfo& ri)
{
    /// Move functionality to scene

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 1.0f, 5.0f));

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f) * 1.0f);

    glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    //prepareShaderParticle(ri.shaderProgram.particle, modelViewMatrix, ri);

    //ri.emitter.updateParticles(ri.time.dt);
    //ri.emitter.renderParticles(ri.shaderProgram.particle);
}
