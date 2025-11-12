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
#include "bulletHelpers.h"

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


// settings 
unsigned int SCR_WIDTH = 3000;
unsigned int SCR_HEIGHT = 1800;

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
    num_x = 0;
    num_y = 0;
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
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-10.0f, 1.0f, 5.0f));
    box->setModelMatrix(modelMatrix);
    scene.addPhongShape(box);
   
    //Shape* pyramid = new Pyramid(2.0, 2.0, 2.0);
    //pyramid->setMaterial(material.ruby);
    //pyramid->useTexture(ri.texture["gray_brick"]);
    //pyramid->castShadow();
    //modelMatrix = glm::mat4(1.0f);
    //modelMatrix = glm::translate(modelMatrix, glm::vec3(-3.0f, 1.3f, -5.0f));
    //pyramid->setModelMatrix(modelMatrix);
    //scene.addPhongShape(pyramid);

    Shape* sphere = new Sphere(1.2);
    sphere->setMaterial(material.brass);
    sphere->castShadow();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-10.0, 5.0, -2.2));
    sphere->setModelMatrix(modelMatrix);
    scene.addPhongShape(sphere);

    //Shape* cylinder = new Cylinder(0.5f, 2.0f, 20);
    //cylinder->useTexture(ri.texture["gray_brick"]);
    //cylinder->castShadow();
    //modelMatrix = glm::mat4(1.0f);
    //modelMatrix = glm::translate(modelMatrix, glm::vec3(6.0f, 5.0f, 0.0f));
    //cylinder->setModelMatrix(modelMatrix);
    //scene.addPhongShape(cylinder);

    //Shape* halfPipe = new HalfPipe(0.9f, 1.0f, 5.0f, 10);
    //halfPipe->useTexture(ri.texture["wood"]);
    //halfPipe->castShadow();
    //modelMatrix = glm::mat4(1.0f);
    //modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 5.0f, 0.0f));
    //halfPipe->setModelMatrix(modelMatrix);
    //scene.addPhongShape(halfPipe);

}

void testBulletShapes(RenderInfo& ri, Scene& scene)
{
    // Plane
    btQuaternion q = quatFromYawPitchRoll(0.0f, 0.0f, 0.0f);
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btRigidBody* planeRigidBody = createStaticRigidBody(
        groundShape, {0, 0, 0}, q, 0.6f, 0.5f);

    ri.bullet.pWorld->addRigidBody(planeRigidBody);

    Shape* plane = new Plane(100, 100);
    //plane->setMaterial(material.emerald);
    plane->useTexture(ri.texture["grass"]);
    plane->setPBody(planeRigidBody);
    scene.addPhongShape(plane);



    // Test sphere
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    btScalar mass = 1.0f;
    btScalar radius = 0.3f;
    btScalar restitution = 0.6f; 
    btScalar friction = 0.8f;

    btRigidBody* sphereRigidBody = createMarbleRigidBody(
        mass, radius, { 0.0f, 4.0f, 0.0f }, restitution, friction);

    ri.bullet.pWorld->addRigidBody(sphereRigidBody);
    ri.camera->setPBody(sphereRigidBody);

    Shape* sphere = new Sphere(radius, 40, 40);
    sphere->setMaterial(material.brass);
    sphere->useTexture(ri.texture["wood"]);
    sphere->castShadow();
    sphere->setPBody(sphereRigidBody);
    scene.addPhongShape(sphere);

    // Emitters
    Emitter* flameEmitter = new FlameEmitter(400, 1.0f, radius * 1.2, 0.1f, ri.texture["particle"]);
    flameEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(flameEmitter);

    Emitter* smokeEmitter = new SmokeEmitter(200, 5.0f, radius * 1.2, 0.05f, ri.texture["particle"]);
    smokeEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(smokeEmitter);



    // Test pyramid: 
    Shape* pyramid = new Pyramid(10.0, 2.5, 10.0);
    
    // Rotate around z, y, x
    q = quatFromYawPitchRoll(0.0f, 30.0f, 0.0f);
    btTriangleMesh* pyramidMesh = createBtTriangleMesh(pyramid);
    btRigidBody* pyramidRigidBody = createStaticRigidBody(
        pyramidMesh, { 21.0, 1.0f, 0.4 }, q, 0.6f, 0.5f);

    ri.bullet.pWorld->addRigidBody(pyramidRigidBody);

    pyramid->setMaterial(material.ruby);
    pyramid->useTexture(ri.texture["gray_brick"]);
    pyramid->castShadow();
    pyramid->setPBody(pyramidRigidBody);
    scene.addPhongShape(pyramid);



    // Test half pipe
    Shape* halfPipe = new HalfPipe(0.6f, 1.0f, 20.5f, 10);

    // Rotate around z, y, x
    q = quatFromYawPitchRoll(0.0f, 90.0f, 2.0f);
    btTriangleMesh* halfPipeMesh = createBtTriangleMesh(halfPipe);
    btRigidBody* halfPipeRigidBody = createStaticRigidBody(
        halfPipeMesh, { 10.0, 2.3f, 0.4 }, q, 0.6f, 0.5f);

    ri.bullet.pWorld->addRigidBody(halfPipeRigidBody);

    halfPipe->useTexture(ri.texture["wood"]);
    halfPipe->castShadow();
    halfPipe->setPBody(halfPipeRigidBody);
    scene.addPhongShape(halfPipe);



    // Test half pipe track
    std::vector<TrackSupport> supports;
    supports.push_back({
        0.0f, 2.0f, 0.0f,   // x, y, z
        0.0f,              // angle
        0.4f,               // inner r
        1.0f                // outer r
        });

    supports.push_back({
        0.0f, 2.0f, 3.0f,
        0.0f, // TODO fix angle
        0.8f,
        1.0f
        });

    supports.push_back({
        -1.0f, 2.0f, 5.0f,
        35.0f, // TODO fix angle
        0.6f,
        1.0f
        });

    Shape* track = new HalfPipeTrack(supports);

    track->useTexture(ri.texture["wood"]);
    track->castShadow();
    //track->setPBody(trackRigidBody);
    scene.addPhongShape(track);

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

