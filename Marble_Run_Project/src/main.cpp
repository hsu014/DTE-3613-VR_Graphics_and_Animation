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
#include "camera.h"
#include "scene.h"

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


void processInput(GLFWwindow* window, RenderInfo& ri);
void initRenderInfo(RenderInfo& ri);
void loadTextures(RenderInfo& ri);
void loadSkyboxTextures(RenderInfo& ri);
void loadHeightmaps(RenderInfo& ri);
void createLights(Scene& scene);
void createShapes(RenderInfo& ri, Scene& scene);
void testBulletShapes(RenderInfo& ri, Scene& scene);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void animate(GLFWwindow* window, RenderInfo& ri, Scene& scene);
void drawScene(Scene& scene, Camera& camera);

void drawEmitter(RenderInfo& ri); // Move to scene later


// settings 
unsigned int SCR_WIDTH = 1600;
unsigned int SCR_HEIGHT = 1200;

const double CAMERA_SPEED = 4;
const double CAMERA_ROT_SPEED = 8;

Utils util = Utils();
Material material{};


int main()
{
    if (!glfwInit()) {
        std::cerr << "Error initializing GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Marble run", NULL, NULL);
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

    Camera camera(window, 
        glm::vec3(0.0f, 1.0f, -10.0f),  // Pos
        glm::vec3(0.0f, 0.1f, 1.0f),    // Front
        glm::vec3(0.0f, 1.0f, 0.0f));   // Up
    ri.camera = &camera;

    Scene scene = Scene(window);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Compile and link shaders
    GLuint shaderProgramBase = Utils::createShaderProgram("src/shader/vertexShaderBase.glsl", "src/shader/fragmentShaderBase.glsl");
    GLuint shaderProgramPhong = Utils::createShaderProgram("src/shader/vertexShaderPhong.glsl", "src/shader/fragmentShaderPhong.glsl");
    GLuint shaderProgramSkybox = Utils::createShaderProgram("src/shader/vertexShaderSkybox.glsl", "src/shader/fragmentShaderSkybox.glsl");
    GLuint shaderProgramShadowMap = Utils::createShaderProgram("src/shader/vertexShaderShadow.glsl", "src/shader/fragmentShaderShadow.glsl");
    GLuint shaderProgramParticle = Utils::createShaderProgram("src/shader/vertexShaderParticle.glsl", "src/shader/fragmentShaderParticle.glsl");
    
    scene.setShaders(shaderProgramBase, shaderProgramPhong, shaderProgramSkybox, shaderProgramShadowMap);
    scene.setParticleShader(shaderProgramParticle);

    Skybox* skybox = new Skybox(ri.skyboxTexture["sky_42"]);
    scene.addSkybox(skybox);
    createLights(scene);
    scene.updateLightSpaceMatrix();

    // Init bullet
    ri.bullet.pCollisionConfiguration = new btDefaultCollisionConfiguration();
    ri.bullet.pDispatcher = new btCollisionDispatcher(ri.bullet.pCollisionConfiguration);
    ri.bullet.pBroadphase = new btDbvtBroadphase();
    ri.bullet.pSolver = new btSequentialImpulseConstraintSolver();
    ri.bullet.pWorld = new btDiscreteDynamicsWorld(
        ri.bullet.pDispatcher, ri.bullet.pBroadphase, ri.bullet.pSolver, ri.bullet.pCollisionConfiguration);
    ri.bullet.pWorld->setGravity(btVector3(0, -9.81f, 0));

    // Create shapes
    createShapes(ri, scene);
    testBulletShapes(ri, scene);

    animate(window, ri, scene);

    // Delete used resources
    glDeleteProgram(shaderProgramBase);
    glDeleteProgram(shaderProgramPhong);
    glDeleteProgram(shaderProgramSkybox);
    glDeleteProgram(shaderProgramShadowMap);
    glDeleteProgram(shaderProgramParticle);

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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void initRenderInfo(RenderInfo& ri)
{
    ri.time.prev = glfwGetTime();
    ri.time.dt = 0;

    loadTextures(ri);
    loadSkyboxTextures(ri);
    loadHeightmaps(ri);
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

void createLights(Scene& scene)
{
    // Ambient
    scene.setAmbientLight(glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

    // Directional
    DirectionalLight dirLight{};
    //dirLight.direction = glm::vec3(0.0f, -5.0f, 3.0f);
    dirLight.direction = glm::vec3(0.2f, -1.0f, 0.8f);

    dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    dirLight.diffuse = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    scene.addDirectionLight(dirLight);

    // Point
    PointLight pointLight{};
    pointLight.position = glm::vec3(-4.0f, 2.0f, 6.0f);

    pointLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    pointLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    scene.addPointLight(pointLight, true);

    // pointLight.position = glm::vec3(-1.0f, 0.1f, 2.0f);
    // scene.addPointLight(pointLight, true);
}


void createShapes(RenderInfo& ri, Scene& scene)
{
    // Create shapes and put into scene
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 middle_pos = glm::vec3(1.0f);
    int num_x;
    int num_y;

    // Boxes:
    middle_pos = { 0.0, 3.0, 0.0 };
    num_x = 10;
    num_y = 4;
    for (int i = 0; i < num_x; i++) {
        for (int j = 0; j < num_y; j++) {
            float x = middle_pos.x - (num_x / 2.0f) + i;
            float y = middle_pos.y - (num_y / 2.0f) + j;
            float z = middle_pos.z;

            Shape* box = new Box(0.5, 0.5, 0.5);
            box->useTexture(ri.texture["wood"]);
            box->castShadow();
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(i * 10.0f), { 0,1,0 });
            box->setModelMatrix(modelMatrix);
            scene.addPhongShape(box);
        }
    }

  
    Shape* box = new Box(2.0, 2.0, 2.0);
    box->setMaterial(material.chrome);
    box->useTexture(ri.texture["gray_brick"]);
    box->castShadow();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.0f, 5.0f));
    box->setModelMatrix(modelMatrix);
    scene.addPhongShape(box);
   
    Shape* pyramid = new Pyramid(2.0, 2.0, 2.0);
    pyramid->setMaterial(material.ruby);
    pyramid->useTexture(ri.texture["gray_brick"]);
    pyramid->castShadow();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-3.0f, 1.3f, -5.0f));
    pyramid->setModelMatrix(modelMatrix);
    scene.addPhongShape(pyramid);

    Shape* sphere = new Sphere(1.2);
    sphere->setMaterial(material.brass);
    sphere->castShadow();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 5.0, -2.2));
    sphere->setModelMatrix(modelMatrix);
    scene.addPhongShape(sphere);

    // Emitter
    //Emitter* flameEmitter = new FlameEmitter(100, 2.0f, 0.5f, 0.1f, ri.texture["particle"]);
    //flameEmitter->setPosition({5,0,0});
    //scene.addEmitter(flameEmitter);

    //Emitter* smokeEmitter = new SmokeEmitter(100, 5.0f, 0.5f, 0.05f, ri.texture["particle"]);
    //smokeEmitter->setPosition({ 5,0,0 });
    //scene.addEmitter(smokeEmitter);

}

void testBulletShapes(RenderInfo& ri, Scene& scene)
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

    Shape* plane = new Plane(50, 50);
    //plane->setMaterial(material.emerald);
    plane->useTexture(ri.texture["grass"]);
    scene.addPhongShape(plane);




    // Test sphere
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    btScalar mass = 1.0f;
    btScalar radius = 0.7f;

    // 1. Create the collision shape
    btCollisionShape* sphereShape = new btSphereShape(radius);

    // 2. Calculate the inertia for the given mass
    btVector3 sphereInertia(0, 0, 0);
    sphereShape->calculateLocalInertia(mass, sphereInertia);

    // 3. Set the initial transform (position & rotation)
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(-2, 2, 2));

    // 4. Create the motion state (keeps transform in sync)
    btDefaultMotionState* sphereMotionState = new btDefaultMotionState(startTransform);

    // 5. Construct the rigid body
    btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(
        mass,
        sphereMotionState,
        sphereShape,
        sphereInertia);

    btRigidBody* sphereRigidBody = new btRigidBody(sphereRigidBodyCI);
    sphereRigidBody->setRestitution(0.6f);      // bounciness
    sphereRigidBody->setFriction(0.8f);         // rolling resistance
    sphereRigidBody->setActivationState(DISABLE_DEACTIVATION);
    //sphereRigidBody->setLinearVelocity(btVector3(0.3f, 0.0f, 0.0f));
    sphereRigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, -3.0f));

    ri.bullet.pWorld->addRigidBody(sphereRigidBody);

    // Use sphereRigidBody for followcam position
    ri.camera->setPBody(sphereRigidBody);

    // Phong shape
    Shape* sphere = new Sphere(radius, 40, 40);
    sphere->setMaterial(material.brass);
    sphere->useTexture(ri.texture["wood"]);
    sphere->castShadow();
    sphere->setPBody(sphereRigidBody);
    scene.addPhongShape(sphere);

    // Emitter
    Emitter* flameEmitter = new FlameEmitter(400, 1.0f, radius * 1.2, 0.1f, ri.texture["particle"]);
    flameEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(flameEmitter);

    Emitter* smokeEmitter = new SmokeEmitter(200, 5.0f, radius * 1.2, 0.05f, ri.texture["particle"]);
    smokeEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(smokeEmitter);

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void animate(GLFWwindow* window, RenderInfo& ri, Scene& scene)
{
    double lastTime = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        ri.time.current = glfwGetTime();
        ri.time.dt = ri.time.current - ri.time.prev;
        ri.time.prev = ri.time.current;

        processInput(window, ri);
        ri.camera->update(ri.time.dt);

        drawScene(scene, *ri.camera);

        ri.bullet.pWorld->stepSimulation(float(ri.time.dt));

        glfwSwapBuffers(window);
        glfwPollEvents();

        // --- FPS tracking ---
        frameCount++;
        //double currentTime = glfwGetTime();
        if (ri.time.current - lastTime >= 1.0) // every 1 second
        {
            double fps = double(frameCount) / (ri.time.current - lastTime);
            std::cout << "FPS: " << fps << std::endl;

            // Optional: show FPS in window title
            std::string title = "OpenGL App - FPS: " + std::to_string(int(fps));
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            lastTime = ri.time.current;
        }

    }
}

void drawScene(Scene& scene, Camera& camera)
{
    // Draw shapes in scene
    scene.update(camera);
    scene.draw();

}

void drawEmitter(RenderInfo& ri)
{
    /// Move functionality to scene

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Translate
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 1.0f, 5.0f));

    // Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f) * 1.0f);

    //glm::mat4 modelViewMatrix = ri.viewMatrix * modelMatrix;

    //prepareShaderParticle(ri.shaderProgram.particle, modelViewMatrix, ri);

    //ri.emitter.updateParticles(ri.time.dt);
    //ri.emitter.renderParticles(ri.shaderProgram.particle);
}
