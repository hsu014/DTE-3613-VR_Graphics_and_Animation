#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <random>
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
void createLights(Scene& scene);

void createTorch(RenderInfo& ri, Scene& scene, glm::vec3 pos);
void createGround(RenderInfo& ri, Scene& scene);
void createSpheres(RenderInfo& ri, Scene& scene, glm::vec3 pos);
void createHalfPipeTrack(RenderInfo& ri, Scene& scene, std::vector<TrackSupport>& supports);
void createPlinko(RenderInfo& ri, Scene& scene, glm::vec3 pos, float angle);

void createWorld(RenderInfo& ri, Scene& scene);

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
        glm::vec3(0.0f, 24.0f, -10.0f), // Pos
        glm::vec3(0.0f, -0.3f, 1.0f),   // Front
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
    // testBulletShapes(ri, scene);
    createWorld(ri, scene);

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
    ri.texture["stone"] = Utils::loadTexture("src/textures/stone2.png");
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

void createTorch(RenderInfo& ri, Scene& scene, glm::vec3 pos)
{
    float height = 0.35f;
    float radius = 0.03f;
    glm::vec4 color = glm::vec4(1.0f, 0.65f, 0.25f, 1.0f);

    PointLight torchLight{
        {pos.x, pos.y + height, pos.z},
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        color * 0.4f,
        glm::vec4(0.5f, 0.45f, 0.35f, 1.0f),
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
    sphere->castShadow(false);
    scene.addBaseShape(sphere);

    scene.addPointLight(torchLight, false);

    Emitter* flameEmitter = new FlameEmitter(400, 0.7f, radius * 1.2, radius * 0.4f, ri.texture["particle"]);
    flameEmitter->setPosition({ pos.x, pos.y + height, pos.z });
    scene.addEmitter(flameEmitter);

    Emitter* smokeEmitter = new SmokeEmitter(100, 2.0f, radius * 1.2, radius * 0.3f, ri.texture["particle"]);
    smokeEmitter->setPosition({ pos.x, pos.y + height, pos.z });
    scene.addEmitter(smokeEmitter);
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

void createSpheres(RenderInfo& ri, Scene& scene, glm::vec3 pos)
{
    std::vector<GLuint> tex = {
        ri.texture["bark"],
        ri.texture["fire"],
        ri.texture["ice"],
        ri.texture["lava"],
        ri.texture["ivy"],
        ri.texture["leaf"],
        ri.texture["gray_brick"],
        ri.texture["pebbles"],
        ri.texture["pebbles2"],
        ri.texture["rock"],
        ri.texture["stone"],
        ri.texture["stone2"],
        ri.texture["tile"],
        ri.texture["tile2"],
        ri.texture["tile3"],
        ri.texture["water"],
        ri.texture["galvanized_blue"],
        ri.texture["metal"],
        ri.texture["metal2"],

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

    std::vector<MaterialType> materials = {
        material.brass,
        //material.bronze,
        //material.polished_bronze,
        //material.chrome,
        material.tin,
        //material.copper,
        //material.polished_copper,
        //material.silver,
        //material.polished_silver,
        material.gold,
        //material.polished_gold,
        material.emerald,
        material.obsidian,
        material.perl,
        material.ruby,
    };

    // generate sphere infos:
    std::vector<SphereInfo> sphereinfo;
    for (int i = 0; i < tex.size(); i++) {
        sphereinfo.push_back({
            MaterialType(),
            tex[i]
        });
    }
    for (int i = 0; i < materials.size(); i++) {
        sphereinfo.push_back({
            materials[i]
            });
    }

    // Shuffle spheres
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(std::begin(sphereinfo), std::end(sphereinfo), g);

    sphereinfo[0].player = true;
    std::shuffle(std::begin(sphereinfo), std::end(sphereinfo), g);

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

                btRigidBody* sphereRigidBody = createMarbleRigidBody(
                    s.mass, s.radius, { xPos, yPos, zPos }, s.restitution, s.friction);

                ri.bullet.pWorld->addRigidBody(sphereRigidBody);

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

void createWorld(RenderInfo& ri, Scene& scene)
{
    // Turn: 90 deg -> 10 segments
    glm::vec3 sphereStartPos = { 0.0f, 20.0f, 0.0f };

    createGround(ri, scene);
    createSpheres(ri, scene, sphereStartPos);

    glm::vec3 nextPos = { 
        sphereStartPos.x,
        sphereStartPos.y,
        sphereStartPos.z - 2.0f,
    };
    float nextAngle = 0.0f;
    std::vector<TrackSupport> supports;
    TrackSupportGenerator trackGenerator = TrackSupportGenerator();

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
    trackGenerator.forward(4.0f, -1.8f, 0.9f, 1.0f);

    trackGenerator.turn(-180.0f, 4.0f, -1.2f, 20);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);

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

    // Track 2a
    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle + 90.0f,
        0.9f, 1.0f);
    trackGenerator.forward(2.0f, -0.1f, 0.9f, 1.0f);
    trackGenerator.turn(-135.0f, 3.0f, -1.0f, 13);
    
    trackGenerator.forward(3.0f, -1.0f, 0.4f, 0.5f);
    trackGenerator.turn(720.0f, 3.0f, -4.0f, 80, 0.4f, 0.5f);
    
    trackGenerator.forward(1.0f, -0.2f, 0.4f, 0.5f);

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

    trackGenerator.forward(3.0f, -1.0f, 0.4f, 0.5f);
    trackGenerator.turn(-720.0f, 3.0f, -4.0f, 80, 0.4f, 0.5f);

    trackGenerator.forward(1.0f, -0.2f, 0.4f, 0.5f);

    supports = trackGenerator.getSupports();
    createHalfPipeTrack(ri, scene, supports);

    glm::vec3 pos2b = trackGenerator.getLastPos();

    // Track 3
    nextPos = (pos2a + pos2b) * 0.5f;
    nextPos.y -= 0.5f;

    createTorch(ri, scene, {
        nextPos.x,
        nextPos.y + 0.5f,
        nextPos.z + 1.5f
        });

    trackGenerator.newTrack(
        nextPos.x, nextPos.y, nextPos.z,
        nextAngle,
        0.9f, 1.0f);
    trackGenerator.forward(6.0f, -1.0f, 0.9f, 1.0f);
    trackGenerator.turn(-180.0f, 8, -2.0f, 20, 0.9f, 1.0f);

    trackGenerator.forward(8.0f, -0.5f, 0.4f, 0.5f);
    trackGenerator.forward(0.5f, 0.1f, 0.4f, 0.5f);
    trackGenerator.forward(0.5f, 0.2f, 0.4f, 0.5f);
    //trackGenerator.turn(-90.0f, 10, -0.5f, 10, 0.4f, 0.5f);

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

void testBulletShapes(RenderInfo& ri, Scene& scene)
{
    // Test pyramid: 
    //Shape* pyramid = new Pyramid(10.0, 2.5, 10.0);
    
    //// Rotate around z, y, x
    //q = quatFromYawPitchRoll(0.0f, 30.0f, 0.0f);
    //btTriangleMesh* pyramidMesh = createBtTriangleMesh(pyramid);
    //btRigidBody* pyramidRigidBody = createStaticRigidBody(
    //    pyramidMesh, { 25.0, 1.0f, 0.4 }, q, 0.6f, 0.5f);

    //ri.bullet.pWorld->addRigidBody(pyramidRigidBody);

    //pyramid->setMaterial(material.ruby);
    //pyramid->useTexture(ri.texture["gray_brick"]);
    //pyramid->setPBody(pyramidRigidBody);
    //scene.addPhongShape(pyramid);



    // Test half pipe
    //Shape* halfPipe = new HalfPipe(0.6f, 1.0f, 20.5f, 10);

    //// Rotate around z, y, x
    //q = quatFromYawPitchRoll(0.0f, 90.0f, 2.0f);
    //btTriangleMesh* halfPipeMesh = createBtTriangleMesh(halfPipe);
    //btRigidBody* halfPipeRigidBody = createStaticRigidBody(
    //    halfPipeMesh, { 13.0, 2.3f, 0.4 }, q, 0.6f, 0.5f);

    //ri.bullet.pWorld->addRigidBody(halfPipeRigidBody);

    //halfPipe->useTexture(ri.texture["wood"]);
    //halfPipe->setPBody(halfPipeRigidBody);
    //scene.addPhongShape(halfPipe);
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

        // Direction light 
        float* lightYaw = &scene.mLightYaw;
        float* lightPitch = &scene.mLightPitch;
        ImGui::Begin("Light", nullptr);
        ImGui::SliderFloat("Yaw", lightYaw, -360.0f, 360.0f);
        ImGui::SliderFloat("Pitch", lightPitch, -89.9f, -10.0f);
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
    scene.update(camera, dt);
    scene.draw();
}

