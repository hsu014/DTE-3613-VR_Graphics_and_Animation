#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

#include "Utils.h"
#include "shape.h"
#include "particle_emitter.h"
#include "render_info.h"
#include "camera.h"
#include "scene.h"
#include "bulletHelpers.h"
#include "trackSupportGenerator.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, RenderInfo& ri);
void initRenderInfo(RenderInfo& ri);
void loadTextures(RenderInfo& ri);
void loadSkyboxTextures(RenderInfo& ri);
void loadHeightmaps(RenderInfo& ri);
void createLights(Scene& scene);
void createShapes(RenderInfo& ri, Scene& scene);
void testBulletShapes(RenderInfo& ri, Scene& scene);

void animate(GLFWwindow* window, RenderInfo& ri, Scene& scene);
void drawScene(Scene& scene, Camera& camera, double dt);


// settings 
unsigned int SCR_WIDTH = 3000;
unsigned int SCR_HEIGHT = 1600;
bool paused = true;
bool pPressedLastFrame = false;

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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW + OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, RenderInfo& ri)
{
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    int pState = glfwGetKey(window, GLFW_KEY_P);
    if (pState == GLFW_PRESS && !pPressedLastFrame) {
        paused = !paused; 
    }
    pPressedLastFrame = (pState == GLFW_PRESS);
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
    ri.texture["particle_star1"] = Utils::loadTexture("src/textures/particle_star1.png");
    ri.texture["particle_star2"] = Utils::loadTexture("src/textures/particle_star2.png");
    ri.texture["particle_star3"] = Utils::loadTexture("src/textures/particle_star3.png");
    ri.texture["grass"] = Utils::loadTexture("src/textures/grass.png");

    ri.texture["bark"] = Utils::loadTexture("src/textures/bark.png");
    ri.texture["fire"] = Utils::loadTexture("src/textures/fire.png");
    ri.texture["galvanized_blue"] = Utils::loadTexture("src/textures/galvanized_blue.jpg");
    ri.texture["gray_brick"] = Utils::loadTexture("src/textures/gray_brick.jpg");
    ri.texture["ice"] = Utils::loadTexture("src/textures/ice.jpg");
    ri.texture["ivy"] = Utils::loadTexture("src/textures/ivy.png");
    ri.texture["lava"] = Utils::loadTexture("src/textures/lava.png");
    ri.texture["leaf"] = Utils::loadTexture("src/textures/leaf.png");
    ri.texture["pebbles"] = Utils::loadTexture("src/textures/pebbles.png");
    ri.texture["pebbles2"] = Utils::loadTexture("src/textures/pebbles2.png");
    ri.texture["rock"] = Utils::loadTexture("src/textures/rock.png");
    ri.texture["tile"] = Utils::loadTexture("src/textures/tile.png");
    ri.texture["tile2"] = Utils::loadTexture("src/textures/tile2.png");
    ri.texture["water"] = Utils::loadTexture("src/textures/water.png");
    ri.texture["wood"] = Utils::loadTexture("src/textures/wood.png");
    
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
    middle_pos = { 0.0f, 5.0f, -4.0f };
    std::vector<GLuint> tex = {
        ri.texture["bark"],
        ri.texture["fire"],
        ri.texture["galvanized_blue"], 
        ri.texture["gray_brick"],
        ri.texture["ice"],
        ri.texture["ivy"],
        ri.texture["lava"],
        ri.texture["leaf"],
        ri.texture["pebbles"],
        ri.texture["pebbles2"],
        ri.texture["rock"],
        ri.texture["tile"],
        ri.texture["tile2"],
        ri.texture["water"],
    };
    num_x = tex.size();
    num_y = 1;
    for (int i = 0; i < num_x; i++) {
        for (int j = 0; j < num_y; j++) {
            float x = middle_pos.x - (num_x / 2.0f) + i;
            float y = middle_pos.y - (num_y / 2.0f) + j;
            float z = middle_pos.z;

            Shape* box = new Sphere(0.4f);
            //Shape* box = new Box(0.5f, 0.5f, 0.5f);
            box->useTexture(tex[i]);
            box->castShadow();
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), { 1,0,0 });
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
    //sphere->setMaterial(material.brass);
    sphere->useTexture(ri.texture["lava"]);
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
    plane->useTexture(ri.texture["grass"]);
    plane->setPBody(planeRigidBody);
    scene.addPhongShape(plane);



    // Test sphere
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    btScalar mass = 1.0f;
    btScalar radius = 0.1f;
    btScalar restitution = 0.6f; 
    btScalar friction = 0.8f;

    btRigidBody* sphereRigidBody = createMarbleRigidBody(
        mass, radius, { 0.0f, 11.0f, 0.0f }, restitution, friction);

    ri.bullet.pWorld->addRigidBody(sphereRigidBody);
    ri.camera->setPBody(sphereRigidBody);

    Shape* sphere = new Sphere(radius, 40, 40);
    sphere->setMaterial(material.brass);
    sphere->useTexture(ri.texture["wood"]);
    sphere->castShadow();
    sphere->setPBody(sphereRigidBody);
    scene.addPhongShape(sphere);

    // Emitters
    Emitter* flameEmitter = new FlameEmitter(400, 0.7f, radius * 1.2, radius * 0.2f, ri.texture["particle"]);
    flameEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(flameEmitter);

    /*Emitter* smokeEmitter = new SmokeEmitter(200, 3.0f, radius * 1.2, radius * 0.3f, ri.texture["particle"]);
    smokeEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(smokeEmitter);*/

    Emitter* trailEmitter = new TrailEmitter(0.1f, 10.0f, radius * 0.5f, ri.texture["particle_star1"]);
    trailEmitter->setPBody(sphereRigidBody);
    scene.addEmitter(trailEmitter);


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

    TrackSupportGenerator trackGenerator = TrackSupportGenerator();
    trackGenerator.newTrack(0.0f, 10.0f, -1.0f, 0.0f);
    trackGenerator.forward(4.0f, -1.0f);
    trackGenerator.turn(-45.0f, 4.0f, 0.0f, 4);

    trackGenerator.forward(1.0f, 0.0f);
    trackGenerator.forward(3.0f, -0.5f, 0.6f, 1.0f);
    trackGenerator.forward(3.0f, -0.5f, 0.8f, 1.0f);
    trackGenerator.forward(1.0f, 0.0f, 0.9f, 1.0f);
    trackGenerator.turn(180.0f, 4.0f, 0.0f, 8);

    trackGenerator.forward(4.0f, -0.5f);
    trackGenerator.turn(765.0f, 4.0f, -6.0f, 50);

    trackGenerator.forward(15.0f, 0.0f);
    trackGenerator.forward(2.0f, 0.5f);
    trackGenerator.forward(0.1f, -0.5f);
    trackGenerator.forward(6.0f, 0.0f);
    trackGenerator.turn(-180.0f, 10.0f, 0.0f, 16);

    trackGenerator.forward(8.0f, -0.4f, 0.5f, 0.6f);

    supports = trackGenerator.getSupports();
    Shape* track = new HalfPipeTrack(supports);

    q = quatFromYawPitchRoll(0.0f, 0.0f, 0.0f);
    btTriangleMesh* trackMesh = createBtTriangleMesh(track);
    btRigidBody* trackRigidBody = createStaticRigidBody(
        trackMesh, { 0, 0, 0 }, q, 0.6f, 0.5f);

    ri.bullet.pWorld->addRigidBody(trackRigidBody);

    track->useTexture(ri.texture["wood"]);
    track->castShadow();
    track->setPBody(trackRigidBody);
    scene.addPhongShape(track);

}


void animate(GLFWwindow* window, RenderInfo& ri, Scene& scene)
{
    double lastTime = glfwGetTime();
    int frameCount = 0;
    double fps = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        ri.time.current = glfwGetTime();
        ri.time.dt = ri.time.current - ri.time.prev;
        ri.time.prev = ri.time.current;

        processInput(window, ri);
        ri.camera->update(ri.time.dt);

        if (paused) {
            ri.time.dt = 0.0;
        }
        drawScene(scene, *ri.camera, ri.time.dt);
        ri.bullet.pWorld->stepSimulation(float(ri.time.dt));

        // FPS 
        frameCount++;
        if (ri.time.current - lastTime >= 1.0) {
            fps = double(frameCount) / (ri.time.current - lastTime);
            frameCount = 0;
            lastTime = ri.time.current;
        }

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 90, 10));
        ImGui::SetNextWindowBgAlpha(0.3f);

        ImGui::Begin("FPS Overlay", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav);
        ImGui::Text("FPS: %.1f", fps);
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}

void drawScene(Scene& scene, Camera& camera, double dt)
{
    // Draw shapes in scene
    scene.update(camera, dt);
    scene.draw();

}

