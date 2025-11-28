#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <random>
#include <cmath>
#include <string>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

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
void createLights(Scene& scene);
glm::mat4 menuSphereModelMatrix(float angle, float radius, glm::vec3 start_pos);
void ImGuiHelpMarker(const char* desc);
void resetCamera(Camera& camera, glm::vec3 start_pos);
void moveCamera(Camera& camera, glm::vec3 pos, glm::vec3 front);

void createTorch(RenderInfo& ri, Scene& scene, glm::vec3 pos);
void createSupportPillar(RenderInfo& ri, Scene& scene, glm::vec3 topPos, float radius);
void createGround(RenderInfo& ri, Scene& scene);
void createSphereInfo(RenderInfo& ri, Scene& scene);
void createSpheres(RenderInfo& ri, Scene& scene, glm::vec3 pos);
void createHalfPipeTrack(RenderInfo& ri, Scene& scene, std::vector<TrackSupport>& supports);
void createPlinko(RenderInfo& ri, Scene& scene, glm::vec3 pos, float angle);
void createFinishLine(RenderInfo& ri, Scene& scene, glm::vec3 pos, float angle, float halfSize, bool visualize);

void createMenuWorld(RenderInfo& ri, Scene& scene);
void createWorld(RenderInfo& ri, Scene& scene);

void createShapes(RenderInfo& ri, Scene& scene);

void animate(GLFWwindow* window, RenderInfo& ri, Scene& scene, Scene& menuScene);
void drawScene(Scene& scene, Camera& camera, double dt);


// Settings 
unsigned int SCR_HEIGHT = 1600;
unsigned int SCR_WIDTH = 3000;
//unsigned int SCR_WIDTH = 1600;
//unsigned int SCR_HEIGHT = 800;
float FONT_SIZE = 17.0f;
glm::vec3 START_POS = { 0.0f, 20.0f, 0.0f };

bool PAUSED = false;
bool P_PRESSED_LAST_FRAME = false;
bool IN_MENU = true;
bool DEBUG = false;

std::vector<std::string> leaderboard;
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

    Camera camera(window);
    resetCamera(camera, START_POS);
    ri.camera = &camera;

    Scene menuScene = Scene(window);
    Scene scene = Scene(window);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Compile and link shaders
    GLuint shaderProgramBase = Utils::createShaderProgram("src/shader/vertexShaderBase.glsl", "src/shader/fragmentShaderBase.glsl");
    GLuint shaderProgramPhong = Utils::createShaderProgram("src/shader/vertexShaderPhong.glsl", "src/shader/fragmentShaderPhong.glsl");
    GLuint shaderProgramSkybox = Utils::createShaderProgram("src/shader/vertexShaderSkybox.glsl", "src/shader/fragmentShaderSkybox.glsl");
    GLuint shaderProgramShadowMap = Utils::createShaderProgram("src/shader/vertexShaderShadow.glsl", "src/shader/fragmentShaderShadow.glsl");
    GLuint shaderProgramParticle = Utils::createShaderProgram("src/shader/vertexShaderParticle.glsl", "src/shader/fragmentShaderParticle.glsl");
    
    menuScene.setShaders(shaderProgramBase, shaderProgramPhong, shaderProgramSkybox, shaderProgramShadowMap);
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
    createSphereInfo(ri, scene);
    createMenuWorld(ri, menuScene);
    // createShapes(ri, scene);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("src/ImGui/misc/fonts/ProggyClean.ttf", FONT_SIZE);
    ImGui::PushFont(myFont);

    // Initialize ImGui for GLFW + OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    animate(window, ri, scene, menuScene);

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
    SCR_WIDTH = std::max(width, 1);
    SCR_HEIGHT = std::max(height, 1);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void processInput(GLFWwindow* window, RenderInfo& ri)
{
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    int pState = glfwGetKey(window, GLFW_KEY_P);
    if (pState == GLFW_PRESS && !P_PRESSED_LAST_FRAME) {
        PAUSED = !PAUSED;
    }
    P_PRESSED_LAST_FRAME = (pState == GLFW_PRESS);
}

void initRenderInfo(RenderInfo& ri)
{
    ri.time.prev = glfwGetTime();
    ri.time.dt = 0;

    loadTextures(ri);
    loadSkyboxTextures(ri);
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

    ri.texture["metal"] = Utils::loadTexture("src/textures/metal.png");
    ri.texture["metal2"] = Utils::loadTexture("src/textures/metal2.png");
    ri.texture["stone"] = Utils::loadTexture("src/textures/stone.png");
    ri.texture["stone2"] = Utils::loadTexture("src/textures/stone2.png");
    ri.texture["tile3"] = Utils::loadTexture("src/textures/tile3.png");

    ri.texture["sun"] = Utils::loadTexture("src/textures/sun.png");
    ri.texture["mercury"] = Utils::loadTexture("src/textures/mercury.png");
    ri.texture["venus"] = Utils::loadTexture("src/textures/venus.png");
    ri.texture["earth_day"] = Utils::loadTexture("src/textures/earth_day.png");
    ri.texture["moon"] = Utils::loadTexture("src/textures/moon.png");
    ri.texture["mars"] = Utils::loadTexture("src/textures/mars.png");
    ri.texture["jupiter"] = Utils::loadTexture("src/textures/jupiter.png");
    ri.texture["saturn"] = Utils::loadTexture("src/textures/saturn.png");
    ri.texture["uranus"] = Utils::loadTexture("src/textures/uranus.png");
    ri.texture["neptune"] = Utils::loadTexture("src/textures/neptune.png");  
}

void loadSkyboxTextures(RenderInfo& ri)
{
    ri.skyboxTexture["sky_22"] = Utils::loadCubeMap("src/textures/skybox/sky_22");
    ri.skyboxTexture["sky_27"] = Utils::loadCubeMap("src/textures/skybox/sky_27");
    ri.skyboxTexture["sky_42"] = Utils::loadCubeMap("src/textures/skybox/sky_42");
}

void createLights(Scene& scene)
{
    // Ambient
    scene.setAmbientLight(glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

    // Directional
    DirectionalLight dirLight{};
    //dirLight.direction = glm::vec3(0.0f, -5.0f, 3.0f);
    dirLight.direction = glm::vec3(1.0f, -1.0f, 0.0f);

    dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    dirLight.diffuse = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    scene.addDirectionLight(dirLight);

    // Point
    /*PointLight pointLight{};
    pointLight.position = glm::vec3(-4.0f, 2.0f, 6.0f);

    pointLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    pointLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    scene.addPointLight(pointLight, true);*/
}

glm::mat4 menuSphereModelMatrix(float angle, float radius, glm::vec3 start_pos)
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float size = 20.0f;

    glm::vec3 pos = {
        start_pos.x,
        start_pos.y + 0.9f,
        start_pos.z - 5.0f
    };

    modelMatrix = glm::translate(modelMatrix, pos);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), { 1, 0, 0 });
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), { 0, 0, 1 });

    modelMatrix = glm::scale(modelMatrix, glm::vec3(size * radius));

    return modelMatrix;
}

void ImGuiHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void resetCamera(Camera& camera, glm::vec3 start_pos)
{
    glm::vec3 camera_pos = {
    start_pos.x,
    start_pos.y + 2,
    start_pos.z - 8
    };

    camera.mPos = camera_pos;
    camera.mFront = glm::vec3(0.0f, -0.3f, 1.0f);
    camera.mUp = glm::vec3(0.0f, 1.0f, 0.0f);

    camera.updatePitchYaw();
}

void moveCamera(Camera& camera, glm::vec3 pos, glm::vec3 front)
{
    camera.mPos = pos;
    camera.mFront = front;
    camera.mUp = glm::vec3(0.0f, 1.0f, 0.0f);

    camera.updatePitchYaw();
}

void createTorch(RenderInfo& ri, Scene& scene, glm::vec3 pos)
{
    float height = 0.35f;
    float radius = 0.03f;
    float intensity = 0.4f;
    glm::vec4 color = glm::vec4(1.0f, 0.65f, 0.25f, 1.0f);

    PointLight torchLight{
        {pos.x, pos.y + height, pos.z},
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        color * intensity,
        glm::vec4(0.5f, 0.45f, 0.35f, 1.0f) * intensity,
        1.0f, 0.09f, 0.032f
    };

    MaterialType mat = {
        color,
        glm::vec4(1.0f),
        glm::vec4(1.0f),
        10.0f
    };

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrixLocal = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, pos);

    Shape* pillar = new Cylinder(radius, height, 20);
    modelMatrixLocal = glm::mat4(1.0f);
    modelMatrixLocal = glm::translate(modelMatrixLocal, {0.0f, height * 0.5f, 0.0f});
    pillar->setModelMatrix(modelMatrix * modelMatrixLocal);
    pillar->useTexture(ri.texture["bark"]);
    scene.addPhongShape(pillar);

    Shape* sphere = new Sphere(radius * 1.1, 20, 20);
    modelMatrixLocal = glm::mat4(1.0f);
    modelMatrixLocal = glm::translate(modelMatrixLocal, { 0.0f, height * 1.0f, 0.0f });
    sphere->setModelMatrix(modelMatrix * modelMatrixLocal);
    sphere->setMaterial(mat);
    scene.addBaseShape(sphere);

    scene.addPointLight(torchLight, false);

    Emitter* flameEmitter = new FlameEmitter(400, 0.7f, radius * 1.2, radius * 0.4f, ri.texture["particle"]);
    flameEmitter->setPosition({ pos.x, pos.y + height, pos.z });
    scene.addEmitter(flameEmitter);

    Emitter* smokeEmitter = new SmokeEmitter(100, 2.0f, radius * 1.2, radius * 0.3f, ri.texture["particle"]);
    smokeEmitter->setPosition({ pos.x, pos.y + height, pos.z });
    scene.addEmitter(smokeEmitter);
}

void createSupportPillar(RenderInfo& ri, Scene& scene, glm::vec3 topPos, float radius)
{
    if (topPos.y <= 0.0f) return;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, { topPos.x, topPos.y/2.0f, topPos.z });

    Shape* pillar = new Cylinder(radius, topPos.y, 20);
    pillar->setModelMatrix(modelMatrix);
    pillar->useTexture(ri.texture["wood"]);
    scene.addPhongShape(pillar);
}

void createGround(RenderInfo& ri, Scene& scene)
{
    btQuaternion q = quatFromYawPitchRoll(0.0f, 0.0f, 0.0f);
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btRigidBody* planeRigidBody = createStaticRigidBody(
        groundShape, { 0, 0, 0 }, q, 0.6f, 0.5f);

    ri.bullet.pWorld->addRigidBody(planeRigidBody);

    Shape* plane = new Plane(100, 100);
    plane->useTexture(ri.texture["grass"]);
    plane->castShadow(false);
    plane->setPBody(planeRigidBody);
    scene.addPhongShape(plane);
}

void createSphereInfo(RenderInfo& ri, Scene& scene)
{
    std::vector<std::tuple<std::string, GLuint>> tex = {
        {"Bark", ri.texture["bark"]},
        {"Fire", ri.texture["fire"]},
        {"Ice", ri.texture["ice"]},
        {"Lava", ri.texture["lava"]},
        {"Ivy", ri.texture["ivy"]},
        {"Leaf", ri.texture["leaf"]},
        {"Gray brick", ri.texture["gray_brick"]},
        {"Pebbles", ri.texture["pebbles"]},
        {"Pebbles 2", ri.texture["pebbles2"]},
        {"Rock", ri.texture["rock"]},
        {"Stones", ri.texture["stone"]},
        {"Stones 2", ri.texture["stone2"]},
        {"Tile", ri.texture["tile"]},
        {"Tile 2", ri.texture["tile2"]},
        {"Tile 3", ri.texture["tile3"]},
        {"Water", ri.texture["water"]},
        {"Galvanized metal", ri.texture["galvanized_blue"]},
        {"Metal", ri.texture["metal"]},
        {"Metal 2", ri.texture["metal2"]},

        {"The Sun", ri.texture["sun"]},
        {"Mercury", ri.texture["mercury"]},
        {"Venus", ri.texture["venus"]},
        {"The Earth", ri.texture["earth_day"]},
        {"The Moon", ri.texture["moon"]},
        {"Mars", ri.texture["mars"]},
        {"Jupiter", ri.texture["jupiter"]},
        {"Saturn", ri.texture["saturn"]},
        {"Uranus", ri.texture["uranus"]},
        {"Neptune", ri.texture["neptune"]},
    };

    std::vector<std::tuple<std::string, MaterialType>> materials = {
        {"Material Brass", material.brass},
        {"Material Tin", material.tin},
        {"Material Gold", material.gold},
        {"Material Emerald", material.emerald},
        {"Material Obsidian", material.obsidian},
        {"Material Perl", material.perl},
        {"Material Ruby", material.ruby},
    };

    std::vector<SphereInfo> sphereinfo;
    for (int i = 0; i < tex.size(); i++) {
        ri.sphereinfo.push_back({
            std::get<0>(tex[i]),
            MaterialType(),
            std::get<1>(tex[i])
            });
    }
    for (int i = 0; i < materials.size(); i++) {
        ri.sphereinfo.push_back({
            std::get<0>(materials[i]),
            std::get<1>(materials[i]),
            });
    }
}

void createSpheres(RenderInfo& ri, Scene& scene, glm::vec3 pos)
{
    std::vector<SphereInfo>& sphereinfo = ri.sphereinfo;

    // Shuffle spheres
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(std::begin(sphereinfo), std::end(sphereinfo), g);

    // Place spheres in world
    int numSpheres = sphereinfo.size();
    int numX = 3;
    int numY = 5;
    int numZ = 3;
    float sphereDist = 0.5;
    float xOffset = -(sphereDist * (numX - 1)) / 2.0f;
    float zOffset = -(sphereDist * (numZ - 1)) / 2.0f;
    float xPos, yPos, zPos;

    for (int i = 0; i < numY; i++) {
        for (int j = 0; j < numZ; j++) {
            for (int k = 0; k < numX; k++)
            {
                int sId = (i * numX * numZ) + (j * numZ) + k;

                if (sId >= numSpheres) return;
                SphereInfo& s = sphereinfo[sId];

                xPos = pos.x + xOffset + (k * sphereDist);
                yPos = pos.y + (i * sphereDist);
                zPos = pos.z + zOffset + (j * sphereDist);

                float mass = s.density * (4.0 / 3.0) * M_PI * std::pow(s.radius, 3.0);

                btRigidBody* sphereRigidBody = createMarbleRigidBody(
                    mass, s.radius, { xPos, yPos, zPos }, s.restitution, s.friction);

                ri.bullet.pWorld->addRigidBody(sphereRigidBody);
                s.pBody = sphereRigidBody;

                Shape* sphere = new Sphere(s.radius, 40, 40);
                sphere->setMaterial(s.material);
                if (s.texture) {
                    sphere->useTexture(s.texture);
                }
                if (s.player) {
                    ri.camera->setPBody(sphereRigidBody);

                    Emitter* trailEmitter = new TrailEmitter(0.1f, 10.0f, s.radius * 0.5f, ri.texture["particle_star1"]);
                    trailEmitter->setPBody(sphereRigidBody);
                    scene.addEmitter(trailEmitter);
                }
                sphere->setPBody(sphereRigidBody);
                scene.addPhongShape(sphere);
            }
        }
    }
}

void createHalfPipeTrack(RenderInfo& ri, Scene& scene, std::vector<TrackSupport>& supports)
{
    Shape* track = new HalfPipeTrack(supports);

    btQuaternion q = quatFromYawPitchRoll(0.0f, 0.0f, 0.0f);
    btTriangleMesh* trackMesh = createBtTriangleMesh(track);
    btRigidBody* trackRigidBody = createStaticRigidBody(
        trackMesh,
        { 0, 0, 0 },
        q, 0.6f, 0.5f);

    ri.bullet.pWorld->addRigidBody(trackRigidBody);

    track->useTexture(ri.texture["wood"]);
    track->setPBody(trackRigidBody);
    scene.addPhongShape(track);
}

void createPlinko(RenderInfo& ri, Scene& scene, glm::vec3 pos, float angle)
{
    float length = 8.0f;
    float width = 4.0f;
    float height = 0.5f;
    float thicknes = 0.1f;
    float radius = 0.05f;
    float tilt = 10.0f;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrixLocal = glm::mat4(1.0f);   // local offset
    btTransform tLocal;                             // local offset bt
    btCompoundShape* compound = new btCompoundShape();

    // Global offset
    modelMatrix = glm::translate(modelMatrix, pos);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-angle), { 0.0f, 1.0f, 0.0f });
    modelMatrix = glm::rotate(modelMatrix, glm::radians(tilt), { 1.0f, 0.0f, 0.0f });

    // base board
    Shape* box1 = new Box(width, thicknes, length);
    modelMatrixLocal = glm::mat4(1.0f);
    box1->setModelMatrix(modelMatrix * modelMatrixLocal);
    box1->useTexture(ri.texture["wood"]);
    scene.addPhongShape(box1);

    btCollisionShape* btBox1 = new btBoxShape(btVector3(width / 2.0f, thicknes / 2.0f, length / 2.0f));
    tLocal.setIdentity();
    tLocal.setOrigin(btVector3(0, 0, 0));
    compound->addChildShape(tLocal, btBox1);


    // right wall
    Shape* box2 = new Box(thicknes, height, length);
    modelMatrixLocal = glm::mat4(1.0f);
    modelMatrixLocal = glm::translate(modelMatrixLocal, { 
        (width/2.0f - thicknes/2.0f), (height/2.0f + thicknes / 2.0f), 0.0f
        });

    box2->setModelMatrix(modelMatrix * modelMatrixLocal);
    box2->useTexture(ri.texture["wood"]);
    scene.addPhongShape(box2);

    btCollisionShape* btBox2 = new btBoxShape(btVector3(thicknes / 2.0f, height / 2.0f, length / 2.0f));
    tLocal.setIdentity();
    tLocal.setOrigin(btVector3((width / 2.0f - thicknes / 2.0f), (height / 2.0f + thicknes / 2.0f), 0.0f));
    compound->addChildShape(tLocal, btBox2);


    // left wall
    Shape* box3 = new Box(thicknes, height, length);
    modelMatrixLocal = glm::mat4(1.0f);
    modelMatrixLocal = glm::translate(modelMatrixLocal, {
        -(width / 2.0f - thicknes / 2.0f), (height / 2.0f + thicknes / 2.0f), 0.0f
        });

    box3->setModelMatrix(modelMatrix * modelMatrixLocal);
    box3->useTexture(ri.texture["wood"]);
    scene.addPhongShape(box3);

    btCollisionShape* btBox3 = new btBoxShape(btVector3(thicknes / 2.0f, height / 2.0f, length / 2.0f));
    tLocal.setIdentity();
    tLocal.setOrigin(btVector3(-(width / 2.0f - thicknes / 2.0f), (height / 2.0f + thicknes / 2.0f), 0.0f));
    compound->addChildShape(tLocal, btBox3);

    // pillar obstacles
    int numRows = 10;
    int pillarsPerRow = 7;
    float pillarDist = 0.625;
    float xOffsetEven = -(pillarDist * (pillarsPerRow - 1)) / 2.0;
    float xOffsetOdd = -(pillarDist * (pillarsPerRow - 2)) / 2.0;
    float xOffset;
    float zOffset = -(pillarDist * (numRows - 1)) / 2.0;
    float xPos, zPos;
    float yPos = (height / 2.0f + thicknes / 2.01f);

    for (int i = 0; i < numRows; i++) {
        if (i % 2 == 0) {
            xOffset = xOffsetEven;
        }
        else {
            xOffset = xOffsetOdd;
        }
        for (int j = 0; j < pillarsPerRow-(i % 2); j++) {
            Shape* pillar = new Cylinder(radius, height, 20);
            xPos = xOffset + (j * pillarDist);
            zPos = zOffset + (i * pillarDist);

            modelMatrixLocal = glm::mat4(1.0f);
            modelMatrixLocal = glm::translate(modelMatrixLocal, { xPos, yPos, zPos });

            pillar->setModelMatrix(modelMatrix * modelMatrixLocal);
            pillar->setMaterial(material.silver);
            scene.addPhongShape(pillar);

            btCollisionShape* btPillar = new btCylinderShape({ radius, height / 2.0f , radius });
            tLocal.setIdentity();
            tLocal.setOrigin(btVector3(xPos, yPos, zPos));
            compound->addChildShape(tLocal, btPillar);
        }
    }

    // Add to bullet world
    btQuaternion q = quatFromYawPitchRoll(0.0f, -angle, tilt);
    btRigidBody* body = createStaticRigidBody(compound, { pos.x, pos.y, pos.z }, q, 0.9f, 0.5f);
    ri.bullet.pWorld->addRigidBody(body);
}

void createFinishLine(RenderInfo& ri, Scene& scene, glm::vec3 pos, float angle, float halfSize, bool visualize)
{
    // test box
    if (visualize) {
        Shape* testBox = new Box(halfSize * 2, halfSize * 2, halfSize * 2);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, pos);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-angle), { 0.0f, 1.0f, 0.0f });

        testBox->setModelMatrix(modelMatrix);
        //testBox->useTexture(ri.texture["rock"]);
        testBox->setMaterial(material.ruby);
        scene.addPhongShape(testBox);
    }

    btQuaternion q = quatFromYawPitchRoll(0.0f, -angle, 0.0f);
    btVector3 finishLinePos = btVector3(pos.x, pos.y, pos.z);

    btGhostObject* ghostObject = new btGhostObject();

    ghostObject->setCollisionShape(new btBoxShape(btVector3(halfSize, halfSize, halfSize)));
    ghostObject->setWorldTransform(btTransform(q, finishLinePos));
    ghostObject->setCollisionFlags(ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

    ri.bullet.pWorld->addCollisionObject(ghostObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::DefaultFilter);

    ri.bullet.pWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    ri.finishLine = ghostObject;
}

void createMenuWorld(RenderInfo& ri, Scene& scene)
{
    //glm::mat4 modelMatrix = glm::mat4(1.0f);
    //modelMatrix = glm::translate(modelMatrix, START_POS);

    Shape* sphere = new Sphere(0.4f);
    sphere->setMaterial(material.brass);
    scene.addPhongShape(sphere);

    // Light
    scene.setAmbientLight(glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

    DirectionalLight dirLight{};
    dirLight.direction = glm::vec3(1.0f, -1.0f, 1.0f);
    dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    dirLight.diffuse = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    scene.addDirectionLight(dirLight);
}

void createWorld(RenderInfo& ri, Scene& scene)
{
    // Track turn: 90 deg -> 10 segments

    glm::vec3 sphereStartPos = START_POS;
    glm::vec3 pillarPos;
    float nextAngle = 0.0f;
    glm::vec3 nextPos = {
        sphereStartPos.x,
        sphereStartPos.y,
        sphereStartPos.z - 2.0f,
    };

    std::vector<TrackSupport> supports;
    TrackSupportGenerator trackGenerator = TrackSupportGenerator();

    createGround(ri, scene);
    createSpheres(ri, scene, sphereStartPos);
    
    // Track 1
    createTorch(ri, scene, { 
        nextPos.x + 1.95f, 
        nextPos.y - 0.05f, 
        nextPos.z + 0.05f
        });
    createTorch(ri, scene, {
        nextPos.x - 1.95f,
        nextPos.y - 0.05f,
        nextPos.z + 0.05f
        });

    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle, 1.9f, 2.0f);
    trackGenerator.forward(4.0f, -2.0f, 1.9f, 2.0f);
    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 1.95f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);
    trackGenerator.forward(4.0f, -1.8f, 0.9f, 1.0f);

    trackGenerator.turn(-180.0f, 4.0f, -1.2f, 20);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.95f;
    pillarPos.z += 0.2f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    // Plinko 1
    nextPos = trackGenerator.nextModuleCenter(8.0f, 10.0f);
    nextAngle = supports.back().angle;
    createPlinko(ri, scene, { nextPos.x, nextPos.y, nextPos.z }, nextAngle);

    // Track 2a & 2b
    nextPos = trackGenerator.nextModuleCenter(16.0f, 10.0f); // After plinko
    
    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z, nextAngle);
    trackGenerator.forward(0.9f);

    nextPos = trackGenerator.getLastPos(); // + offset
    pillarPos = nextPos;
    pillarPos.y -= 0.95f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    // Track 2a
    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle + 90.0f,
        0.9f, 1.0f);
    trackGenerator.forward(2.0f, -0.1f, 0.9f, 1.0f);
    trackGenerator.turn(-135.0f, 3.0f, -1.0f, 13);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.95f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);
    
    trackGenerator.forward(3.0f, -1.0f, 0.4f, 0.5f);
    trackGenerator.turn(720.0f, 3.0f, -4.0f, 80, 0.4f, 0.5f);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.45f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);
    
    trackGenerator.forward(1.0f, -0.2f, 0.3f, 0.4f);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);

    glm::vec3 pos2a = trackGenerator.getLastPos();

    // Track 2b
    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle - 90.0f,
        0.9f, 1.0f);
    trackGenerator.forward(2.0f, -0.1f, 0.9f, 1.0f);
    trackGenerator.turn(135.0f, 3.0f, -1.0f, 13);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.95f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    trackGenerator.forward(3.0f, -1.0f, 0.4f, 0.5f);
    trackGenerator.turn(-720.0f, 3.0f, -4.0f, 80, 0.4f, 0.5f);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.45f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    trackGenerator.forward(1.0f, -0.2f, 0.3f, 0.4f);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);

    glm::vec3 pos2b = trackGenerator.getLastPos();

    // Track 3
    nextPos = (pos2a + pos2b) * 0.5f;
    nextPos.y -= 0.5f;

    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle,
        0.9f, 1.0f);
    trackGenerator.forward(6.0f, -1.0f, 0.9f, 1.0f);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.95f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    trackGenerator.turn(-180.0f, 8, -2.0f, 20, 0.9f, 1.0f);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.95f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    trackGenerator.forward(8.0f, -0.5f, 0.4f, 0.5f);
    trackGenerator.forward(0.5f, 0.1f, 0.4f, 0.5f);
    trackGenerator.forward(0.5f, 0.2f, 0.4f, 0.5f);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);

    pillarPos = trackGenerator.getLastPos();

    // Finish line
    float lastRadius = 0.5f;
    float finishAngle = supports.back().angle;

    trackGenerator.forward(lastRadius + 0.1f);
    glm::vec3 finishPos = trackGenerator.getLastPos();

    createFinishLine(ri, scene, finishPos, finishAngle, lastRadius, false);

    pillarPos.x += 1.1f;
    createTorch(ri, scene, pillarPos);
    createSupportPillar(ri, scene, pillarPos, 0.1f);

    pillarPos.x -= 2.2f;
    createTorch(ri, scene, pillarPos);
    createSupportPillar(ri, scene, pillarPos, 0.1f);

    // Landing area after race
    trackGenerator.forward(0.0f, -0.5f);
    nextPos = trackGenerator.getLastPos();
    nextAngle = finishAngle;

    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle, 0.9f, 1.0f);
    trackGenerator.forward(10.0f, -1.0f, 0.2f, 0.3f);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.25f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    trackGenerator.forward(20.0f, 0.0f, 0.2f, 0.3f);

    pillarPos = trackGenerator.getLastPos();
    pillarPos.y -= 0.25f;
    createSupportPillar(ri, scene, pillarPos, 0.2f);

    trackGenerator.forward(4.0f, 1.2f, 0.2f, 0.3f);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);
}

void createShapes(RenderInfo& ri, Scene& scene)
{
    // Create shapes and put into scene
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 middle_pos = glm::vec3(1.0f);
    int num_x;
    int num_y;

    // Boxes:
    middle_pos = { -10.0f, 5.0f, -4.0f };
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

        ri.texture["sun"],
        ri.texture["mercury"],
        ri.texture["venus"],
        ri.texture["earth_day"],
        ri.texture["moon"],
        ri.texture["mars"],
        ri.texture["jupiter"],
        ri.texture["saturn"],
        ri.texture["uranus"],
        ri.texture["neptune"], 
    };
    num_x = tex.size();
    for (int i = 0; i < num_x; i++) {
        float x = middle_pos.x - (num_x / 2.0f) + i;
        float y = middle_pos.y;
        float z = middle_pos.z;

        Shape* sphere = new Sphere(0.4f);
        sphere->useTexture(tex[i]);
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), { 1,0,0 });
        sphere->setModelMatrix(modelMatrix);
        scene.addPhongShape(sphere);
    }

    std::vector<MaterialType> materials = {
        material.brass,
        material.bronze,
        material.polished_bronze,
        material.chrome,
        material.tin,
        material.copper,
        material.polished_copper,
        material.silver,
        material.polished_silver,
        material.gold,
        material.polished_gold,
        material.emerald,
        material.obsidian,
        material.perl,
        material.ruby,
    };

    middle_pos = { -10.0f, 5.0f, -1.0f };
    num_x = materials.size();
    for (int i = 0; i < num_x; i++) {
        float x = middle_pos.x - (num_x / 2.0f) + i;
        float y = middle_pos.y;
        float z = middle_pos.z;

        Shape* sphere = new Sphere(0.4f);
        sphere->setMaterial(materials[i]);
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), { 1,0,0 });
        sphere->setModelMatrix(modelMatrix);
        scene.addPhongShape(sphere);
    }

}

void animate(GLFWwindow* window, RenderInfo& ri, Scene& scene, Scene& menuScene)
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

        if (PAUSED) {
            ri.time.dt = 0.0;
        }

        if (IN_MENU) {
            ri.camera->mAcceptInput = false;
            drawScene(menuScene, *ri.camera, ri.time.dt);
        }
        else {
            ri.camera->mAcceptInput = true;
            drawScene(scene, *ri.camera, ri.time.dt);
            ri.bullet.pWorld->stepSimulation(float(ri.time.dt));
            static int placement = 1;

            int numOverlapping = ri.finishLine->getNumOverlappingObjects();
            for (int i = 0; i < numOverlapping; i++) {
                btCollisionObject* otherObject = ri.finishLine->getOverlappingObject(i);
                btRigidBody* otherBody = btRigidBody::upcast(otherObject);
                if (!otherBody) continue;

                // Find iterator to current sphere
                auto it = std::find_if(
                    ri.sphereinfo.begin(),
                    ri.sphereinfo.end(),
                    [otherBody](const SphereInfo& info) {
                        return info.pBody == otherBody;
                    }
                );
                if (it == ri.sphereinfo.end()) continue;

                SphereInfo& sphere = *it;

                if (sphere.placement == 0) {
                    sphere.placement = placement++;
                    //std::cout << (sphere.player ? "Player" : sphere.description) <<
                    //   " finished " << sphere.placement << std::endl;

                    leaderboard.push_back(sphere.player ? "**Player**" : sphere.description);
                }
            }
        }

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

        // Show fps
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 90 * 1.7f, 10));
        ImGui::SetNextWindowBgAlpha(0.3f);
        ImGui::Begin("FPS Overlay", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNav);
        ImGui::Text("FPS: %.1f", fps);
        ImGui::SetWindowFontScale(1.2f);
        ImGui::End();

        // Direction light controls
        float* lightYaw = &scene.mLightYaw;
        float* lightPitch = &scene.mLightPitch;
        if (IN_MENU) {
            lightYaw = &menuScene.mLightYaw;
            lightPitch = &menuScene.mLightPitch;
        }

        ImGui::SetNextWindowSize(
            ImVec2(SCR_WIDTH * 0.2f, SCR_HEIGHT * 0.4),
            ImGuiCond_FirstUseEver);

        ImGui::Begin("ImGui", nullptr,
            ImGuiWindowFlags_NoSavedSettings);

        if (ImGui::CollapsingHeader("Direction Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Yaw", lightYaw, -360.0f, 360.0f);
            ImGui::SliderFloat("Pitch", lightPitch, -89.9f, -10.0f);
        }
        
        // Controlls
        if (!IN_MENU) {
            ImGui::Spacing();
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Shadow", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Shadow area", &scene.mShadowAreaSize, 10.0f, 100.0f);
            }

            ImGui::Spacing();
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SeparatorText("General");
                ImGui::Text("[esc] - - - Free/lock mouse cursor");
                ImGui::Text("[p] - - - - Pause/unpause game");
                ImGui::Text("[backspace] Close application");
                ImGui::Spacing();
                ImGui::SeparatorText("Free cam");
                ImGui::Text("[1]- - - - - - Activate freecam");
                ImGui::Text("[mouse]- - - - Camera direction ");
                ImGui::Text("[W, A, S, D] - Camera position ");
                ImGui::Text("[space, shift] Camera height ");
                ImGui::Spacing();
                ImGui::SeparatorText("Follow cam");
                ImGui::Text("[2]- - - - - - Activate followcam");
                ImGui::Text("[mouse]- - - - Camera direction ");
                ImGui::Text("[W, S] - - - - Camera zoom ");
                ImGui::Text("[space]- - - - Disable followcam ");

                if (DEBUG) {
                    glm::vec3* pos = &ri.camera->mPos;
                    glm::vec3* front = &ri.camera->mFront;

                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::SeparatorText("Debug camera info");
                    ImGui::Text("pos:   %.1f %.1f %.1f", pos->x, pos->y, pos->z);
                    ImGui::Text("front: %.1f %.1f %.1f", front->x, front->y, front->z);
                }
            }
        }

        // Menu
        if (IN_MENU) {
            Shape* menuSphere = menuScene.mPhongShapes.front();

            static float sphereAngle = 0.0f;
            static float sphereRadius = 0.1f;
            static float sphereDensity = 1.0f;

            // Rotating
            sphereAngle += ri.time.dt * 12.0f;
            if (sphereAngle > 360.0f) sphereAngle -= 360.0f;

            glm::mat4 modelMatrix = menuSphereModelMatrix(sphereAngle, sphereRadius, START_POS);
            menuSphere->setModelMatrix(modelMatrix);

            ImGui::Spacing();
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Edit marble", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Radius", &sphereRadius, 0.08f, 0.12f);
                ImGui::SameLine(); ImGuiHelpMarker("Default radius = 0.1");
                ImGui::SliderFloat("Density", &sphereDensity, 0.1f, 10.0f);
                ImGui::SameLine(); ImGuiHelpMarker("Default density = 1.0");

                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Text("Select marble");
                ImGui::Spacing();
                if (ImGui::BeginListBox("##Select marble", ImVec2(0, 300))) {
                    static int selectedSphereIdx = 0;

                    for (int i = 0; i < ri.sphereinfo.size(); i++) {
                        const bool isSelected = (selectedSphereIdx == i);
                        if (ImGui::Selectable(ri.sphereinfo[i].description.c_str(), isSelected)) {
                            selectedSphereIdx = i;
                        }

                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();

                    menuSphere->setMaterial(ri.sphereinfo[selectedSphereIdx].material);
                    menuSphere->useTexture(ri.sphereinfo[selectedSphereIdx].texture);

                    ImGui::Spacing();
                    ImGui::Spacing();
                    // Start marble run
                    if (ImGui::Button("Confirm marble selection")) {
                        IN_MENU = false;
                        PAUSED = true;
                        ri.sphereinfo[selectedSphereIdx].player = true;
                        ri.sphereinfo[selectedSphereIdx].radius = sphereRadius;
                        ri.sphereinfo[selectedSphereIdx].density = sphereDensity;
                        ri.camera->captureMouse();

                        createWorld(ri, scene);

                        // Set camera
                        //moveCamera(*ri.camera,
                        //    {-4.1, 3.8, -8.7 },
                        //    {-0.9, -0.2, 0.4});
                        //ri.camera->mMouseLocked = false;
                        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    }
                }
            }
        }
        ImGui::End();

        // Leaderboard window
        if (!IN_MENU) {
            
            ImGui::SetNextWindowSize(
                ImVec2(SCR_WIDTH * 0.1f, SCR_HEIGHT * 0.4),
                ImGuiCond_FirstUseEver);

            ImGui::SetNextWindowPos(ImVec2(
                SCR_WIDTH * 0.9f - 20.0f, 80.0f));

            ImGui::Begin("Leaderboard", nullptr,
                ImGuiWindowFlags_NoSavedSettings);

            if (ImGui::BeginTable("Placement", 2)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, SCR_WIDTH * 0.07f);
                ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthStretch, 1.0f);

                for (int i = 0; i < leaderboard.size(); i++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", leaderboard[i].c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", i+1);

                }
                ImGui::EndTable();
            } 
            ImGui::End();
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void drawScene(Scene& scene, Camera& camera, double dt)
{
    scene.update(camera, dt);
    scene.draw();
}

