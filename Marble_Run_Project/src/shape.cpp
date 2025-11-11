#include "shape.h"

Shape::~Shape()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(4, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Shape::initBuffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(4, VBO);
    glGenBuffers(1, &EBO);
}

void Shape::fillVertexBuffer(std::vector<float> vertices)
{
    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Shape::fillColorBuffer(std::vector<float> colors)
{
    // color attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

void Shape::fillUVBuffer(std::vector<float> textureUVs)
{
    // texture UV attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, textureUVs.size() * sizeof(float), &textureUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
}

void Shape::fillNormalBuffer(std::vector<float> normals)
{ 
    // normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
}

void Shape::fillIndexBuffer(std::vector<unsigned int> indices)
{
    mIndexCount = static_cast<GLsizei>(indices.size());
    // index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
}

void Shape::setModelMatrix(glm::mat4 modelMatrix)
{
    mModelMatrix = modelMatrix;
}

void Shape::useTexture(GLuint texture)
{
    mTexture = texture;
}

void Shape::setMaterial(MaterialType mat)
{
    mAmbient = mat.ambient;
    mDiffuse = mat.diffuse;
    mSpecular = mat.specular;
    mShininess = mat.shininess;
}

void Shape::setPBody(btRigidBody* pBody)
{
    m_pBody = pBody;
}

void Shape::castShadow(bool castShadow)
{
    mCastShadow = castShadow;
}

void Shape::draw(GLuint shaderProgram)
{
    if (m_pBody) {
        btTransform trans;
        m_pBody->getMotionState()->getWorldTransform(trans);

        btScalar matrix[16];
        trans.getOpenGLMatrix(matrix);
        
        mModelMatrix = glm::make_mat4(matrix);
    }

    shaderSetMat4(shaderProgram, "uModel", mModelMatrix);
    glBindVertexArray(VAO);

    if (mTexture)
    {
        shaderSetInt(shaderProgram, "useTexture", 1);
        glActiveTexture(GL_TEXTURE0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, mTexture); 
    }
    else {
        shaderSetInt(shaderProgram, "useTexture", 0);
    }

    // Material
    shaderSetVec4(shaderProgram, "material.ambient", mAmbient);
    shaderSetVec4(shaderProgram, "material.diffuse", mDiffuse);
    shaderSetVec4(shaderProgram, "material.specular", mSpecular);
    shaderSetFloat(shaderProgram, "material.shininess", mShininess);

    // Normal matrix
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(mModelMatrix));
    shaderSetMat4(shaderProgram, "uNormal", normalMatrix);

    glEnable(GL_CULL_FACE);

    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}



Skybox::Skybox(GLuint texture)
{
    mTexture = texture;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, VBO);

    fillBuffers();
}

void Skybox::fillBuffers()
{
    std::vector<float> vertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);

    glBindVertexArray(0);
}

void Skybox::draw(GLuint shaderProgram)
{
    glBindVertexArray(VAO);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);	// cube is CW, but we are viewing the inside

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    glBindVertexArray(0);
}



Box::Box(float size_x, float size_y, float size_z) :
    mSizeX(size_x), mSizeY(size_y), mSizeZ(size_z)
{
    initBuffers();
    fillBuffers();
}

void Box::fillBuffers()
{
    const float x = mSizeX / 2.0f;
    const float y = mSizeY / 2.0f;
    const float z = mSizeZ / 2.0f;

    std::vector<float> vertices = {
        // Front face
        -x, -y,  z,   
        -x, -y, -z,   
        -x,  y, -z,   
        -x,  y,  z,   

        // Back face
         x, -y, -z,   
         x, -y,  z,   
         x,  y,  z,   
         x,  y, -z,   

        // Right face
         x, -y,  z,   
        -x, -y,  z,   
        -x,  y,  z,   
         x,  y,  z,   

        // Left face
        -x, -y, -z,  
         x, -y, -z,  
         x,  y, -z,  
        -x,  y, -z,  

        // Top face
        -x,  y,  z,  
        -x,  y, -z,  
         x,  y, -z,  
         x,  y,  z,  

        // Bottom face
         x, -y, -z,  
         x, -y,  z,  
        -x, -y,  z,  
        -x, -y, -z,  
    };

    std::vector<float> colors = {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.4f, 0.0f,
        1.0f, 0.7f, 0.0f,
        1.0f, 0.4f, 0.0f,

        1.0f, 0.0f, 0.0f,
        1.0f, 0.4f, 0.0f,
        1.0f, 0.7f, 0.0f,
        1.0f, 0.4f, 0.0f,

        1.0f, 0.4f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.4f, 0.0f,
        1.0f, 0.7f, 0.0f,

        1.0f, 0.4f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.4f, 0.0f,
        1.0f, 0.7f, 0.0f,

        1.0f, 0.4f, 0.0f,
        1.0f, 0.7f, 0.0f,
        1.0f, 0.4f, 0.0f,
        1.0f, 0.7f, 0.0f,

        1.0f, 0.0f, 0.0f,
        1.0f, 0.4f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.4f, 0.0f,
    };

    std::vector<float> textureUVs{
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,

    };

    std::vector<float> normals = {
        // Front face
        -1.0f, 0.0f, 0.0f, 
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 
        -1.0f, 0.0f, 0.0f,
        // Back face
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        // Right face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        // Left face
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        // Top face
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // Bottom face
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
    };

    std::vector<unsigned int> indices = {
        // Front face
        0, 2, 1,
        2, 0, 3,
        // Back face
        4, 6, 5,
        6, 4, 7,
        // Right face
        8, 10, 9,
        10, 8, 11,
        // Left face
        12, 14, 13,
        14, 12, 15,
        // Top face
        16, 18, 17,
        18, 16, 19,
        // Bottom face
        20, 21, 22,
        22, 23, 20,
    };


    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    // Unbind VAO
    glBindVertexArray(0);

};



Pyramid::Pyramid(float size_x, float height, float size_z) :
    mSizeX(size_x), mHeight(height), mSizeZ(size_z)
{
    initBuffers();
    fillBuffers();
}

void Pyramid::fillBuffers()
{
    const float x = mSizeX / 2.0f;
    const float h = mHeight / 2.0f;
    const float z = mSizeZ / 2.0f;

    std::vector<float> vertices = {
        // Side 1
        -x, -h, -z,
         x, -h, -z,
         0,  h,  0,

        // Side 2
         x, -h, -z,
         x, -h,  z,
         0,  h,  0,

        // Side 3
         x, -h, z, 
        -x, -h, z, 
         0,  h, 0, 

        // Side 4 
        -x, -h,  z,
        -x, -h, -z,
         0,  h,  0,

        // Bottom    
         x, -h, -z,
         x, -h,  z,
        -x, -h,  z,
        -x, -h, -z,
    };

    std::vector<float> colors{
         0.0f, 0.0f, 1.0f,
         0.0f, 1.0f, 0.0f,
         1.0f, 0.0f, 0.0f,

         0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         1.0f, 0.0f, 0.0f,

         0.0f, 0.0f, 1.0f,
         0.0f, 1.0f, 0.0f,
         1.0f, 0.0f, 0.0f,

         0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         1.0f, 0.0f, 0.0f,

         0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
    };

    std::vector<float> textureUVs = {
        1.0, 0.0,
        0.0, 0.0,
        0.5, 1.0,

        1.0, 0.0,
        0.0, 0.0,
        0.5, 1.0,

        1.0, 0.0,
        0.0, 0.0,
        0.5, 1.0,

        1.0, 0.0,
        0.0, 0.0,
        0.5, 1.0,

        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0,
    };

    glm::vec3 n1 = glm::normalize(glm::cross(glm::vec3(0, mHeight, z), glm::vec3(1, 0, 0) ));
    glm::vec3 n2 = glm::normalize(glm::cross(glm::vec3(-x, mHeight, 0), glm::vec3(0, 0, 1) ));
    glm::vec3 n3 = glm::normalize(glm::cross(glm::vec3(0, mHeight, -z), glm::vec3(-1, 0, 0) ));
    glm::vec3 n4 = glm::normalize(glm::cross(glm::vec3(x, mHeight, 0), glm::vec3(0, 0, -1) ));
 
    std::vector<float> normals = {
        n1.x, n1.y, n1.z,
        n1.x, n1.y, n1.z,
        n1.x, n1.y, n1.z,

        n2.x, n2.y, n2.z,
        n2.x, n2.y, n2.z,
        n2.x, n2.y, n2.z,

        n3.x, n3.y, n3.z,
        n3.x, n3.y, n3.z,
        n3.x, n3.y, n3.z,

        n4.x, n4.y, n4.z,
        n4.x, n4.y, n4.z,
        n4.x, n4.y, n4.z,

        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
    };

    std::vector<unsigned int> indices = {
        // Sides
        0, 2, 1,
        3, 5, 4,
        6, 8, 7,
        9, 11, 10,
        // Bottom
        12, 13, 14,
        14, 15, 12,
    };

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    mVertices = vertices;
    mIndices = indices;

    // Unbind VAO
    glBindVertexArray(0);
}



Plane::Plane(float size_x, float size_z) :
    mSizeX(size_x), mSizeZ(size_z)
{
    initBuffers();
    fillBuffers();
}

void Plane::fillBuffers()
{
    const float x = mSizeX / 2.0f;
    const float z = mSizeZ / 2.0f;

    std::vector<float> vertices = {  
             x, 0.0f, -z,
             x, 0.0f,  z,
            -x, 0.0f,  z,
            -x, 0.0f, -z,
    };

    std::vector<float> colors{
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
    };

    std::vector<float> textureUVs = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0,
    };

    std::vector<float> normals = {
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
    };

    std::vector<unsigned int> indices = {
        0, 2, 1,
        2, 0, 3,
    };

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    // Unbind VAO
    glBindVertexArray(0);
}



CompositePlane::CompositePlane(int width, int depth, GLuint texture)
    : mWidth(width), mDepth(depth), mHeightMap(nullptr)
{
    mTexture = texture;
    initBuffers();
    fillBuffers();
}

CompositePlane::CompositePlane(
    GLuint texture,
    std::shared_ptr<std::vector<std::vector<float>>> heightMap)
    : mHeightMap(heightMap)
{
    mTexture = texture;

    if (mHeightMap && !mHeightMap->empty() && !(*mHeightMap)[0].empty()) {
        mWidth = (*mHeightMap).size();
        mDepth = (*mHeightMap)[0].size();
    }
    else {
        std::cout << "Heightmap is empty or not assigned.\n";
    }

    initBuffers();
    fillBuffers();
}

void CompositePlane::fillBuffers()
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> textureUVs;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    const float x_to_z_ratio = static_cast<float>(mDepth) / mWidth;
    const float scale = 0.1f;


    for (int x = 0; x < mWidth; x++) {
        for (int z = 0; z < mDepth; z++) {

            float u = (x / float(mWidth - 1));
            float v = (z / float(mDepth - 1));

            float height = 0.0f;
            if (mHeightMap && !mHeightMap->empty() && !(*mHeightMap)[0].empty()) {
                height = (*mHeightMap)[x][z] * scale; // safe access
            }

            vertices.push_back(u - 0.5f);
            vertices.push_back(height);
            vertices.push_back((v - 0.5f) * x_to_z_ratio);

            // color
            float c = 1.0f; // (static_cast<float>(x + z)) / (mWidth + mDepth);
            colors.push_back(c);
            colors.push_back(c);
            colors.push_back(c);

            // UV
            textureUVs.push_back(1-u); // Mirror texture the correct way
            textureUVs.push_back(v);

            normals.push_back(0.0f);
            normals.push_back(1.0f);
            normals.push_back(0.0f);
        }
    }

    for (int x = 0; x < mWidth -1; x++) {
        for (int z = 0; z < mDepth -1; z++) {

            int start = x * mDepth + z;

            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + mDepth);

            indices.push_back(start + 1);
            indices.push_back(start + mDepth + 1);
            indices.push_back(start + mDepth);
        }
    }

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    // Unbind VAO
    glBindVertexArray(0);
}



Sphere::Sphere(float radius, int sectors, int stacks) : 
    mRadius(radius), mSectors(sectors), mStacks(stacks)
{
    initBuffers();
    fillBuffers();
}

void Sphere::fillBuffers()
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> textureUVs;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    const float radius = mRadius;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float u, v;                                     // texCoord

    const float sectorStep = 2 * PI / mSectors;
    const float stackStep = PI / mStacks;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= mStacks; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= mSectors; ++j) {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);

            // vertex tex coord between [0, 1]
            u = (float)j / mSectors;
            v = (float)i / mStacks;

            textureUVs.push_back(u);
            textureUVs.push_back(v);

            // color
            colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < mStacks; ++i) {
        k1 = i * (mSectors + 1);     // beginning of current stack
        k2 = k1 + mSectors + 1;      // beginning of next stack

        for (int j = 0; j < mSectors; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding 1st and last stacks
            if (i != 0) {
                //addIndices(k1, k2, k1 + 1);   // k1---k2---k1+1
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (mStacks - 1)) {
                //addIndices(k1 + 1, k2, k2 + 1); // k1+1---k2---k2+1
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }

            // vertical lines for all stacks
            //lineIndices.push_back(k1);
            //lineIndices.push_back(k2);
            //if (i != 0)  // horizontal lines except 1st stack
            //{
            //    lineIndices.push_back(k1);
            //    lineIndices.push_back(k1 + 1);
            //}
        }
    }

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    // Unbind VAO
    glBindVertexArray(0);
}



Cylinder::Cylinder(float radius, float height, int sectors) : 
    mRadius(radius), mHeight(height), mSectors(sectors)
{
    initBuffers();
    fillBuffers();
}

void Cylinder::fillBuffers()
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> textureUVs;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    const float radius = mRadius;
    const float h = mHeight / 2;

    float x, z;                                     // vertex position
    float nx, ny, nz;                               // normal
    float u, v;                                     // texCoord

    const float sectorStep = 2 * PI / mSectors;
    float sectorAngle;

    // Outer shell:
    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = i * sectorStep;

        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);
        x = radius * cosA;
        z = radius * sinA;
        nx = cosA;
        ny = 0.0f;
        nz = sinA;

        // Top vertex
        vertices.push_back(x);
        vertices.push_back(h);
        vertices.push_back(z);

        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(nz);

        u = (float)i / mSectors;
        v = 1.0f;
        textureUVs.push_back(u);
        textureUVs.push_back(v);

        colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });

        // Bottom vertex
        vertices.push_back(x);
        vertices.push_back(-h);
        vertices.push_back(z);

        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(nz);

        v = 0.0f;
        textureUVs.push_back(u);
        textureUVs.push_back(v);

        colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });
    }

    for (int i = 0; i < mSectors; ++i) {
        int k1 = i * 2;
        int k2 = k1 + 2;

        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);

        indices.push_back(k2);
        indices.push_back(k2 + 1);
        indices.push_back(k1 + 1);
    }

    // Top circle:
    int topCenterIndex = vertices.size() / 3;
    vertices.push_back(0.0f); 
    vertices.push_back(h); 
    vertices.push_back(0.0f);

    normals.push_back(0.0f); 
    normals.push_back(1.0f); 
    normals.push_back(0.0f);

    textureUVs.push_back(0.5f); 
    textureUVs.push_back(0.5f);

    colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });

    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = i * sectorStep;
        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);
        x = radius * cosA;
        z = radius * sinA;

        vertices.push_back(x);
        vertices.push_back(h);
        vertices.push_back(z);

        normals.push_back(0.0f);
        normals.push_back(1.0f);
        normals.push_back(0.0f);

        textureUVs.push_back((cosA + 1.0f) * 0.5f);
        textureUVs.push_back((sinA + 1.0f) * 0.5f);

        colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });
    }

    for (int i = 0; i < mSectors; ++i) {
        indices.push_back(topCenterIndex);
        indices.push_back(topCenterIndex + i + 2);
        indices.push_back(topCenterIndex + i + 1);
    }

    // Bottom circle:
    int bottomCenterIndex = vertices.size() / 3;

    vertices.push_back(0.0f); 
    vertices.push_back(-h); 
    vertices.push_back(0.0f);

    normals.push_back(0.0f); 
    normals.push_back(-1.0f); 
    normals.push_back(0.0f);

    textureUVs.push_back(0.5f); 
    textureUVs.push_back(0.5f);

    colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });

    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = i * sectorStep;
        float cosA = cos(sectorAngle);
        float sinA = sin(sectorAngle);
        x = radius * cosA;
        z = radius * sinA;

        vertices.push_back(x);
        vertices.push_back(-h);
        vertices.push_back(z);

        normals.push_back(0.0f);
        normals.push_back(-1.0f);
        normals.push_back(0.0f);

        textureUVs.push_back((cosA + 1.0f) * 0.5f);
        textureUVs.push_back((sinA + 1.0f) * 0.5f);

        colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f });
    }

    for (int i = 0; i < mSectors; ++i) {
        indices.push_back(bottomCenterIndex);
        indices.push_back(bottomCenterIndex + i + 1);
        indices.push_back(bottomCenterIndex + i + 2);
    }

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    // Unbind VAO
    glBindVertexArray(0);
}



HalfPipe::HalfPipe(float innerRadius, float outerRadius, float length, int sectors) :
    mInnerRadius(innerRadius), mOuterRadius(outerRadius), mLength(length), mSectors(sectors)
{
    initBuffers();
    fillBuffers();
}

void HalfPipe::fillBuffers()
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> textureUVs;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    const float rInner = mInnerRadius;
    const float rOuter = mOuterRadius;
    const float length = mLength / 2.0f;

    float xIn, xOut, yIn, yOut;                     // vertex position
    float nx, ny, nz;                               // normal
    float u, v;                                     // texCoord
    const float uvInner = rInner / rOuter;          // uv distance for inner curve [0 to 1]

    const float sectorStep = PI / mSectors;
    float sectorAngle;
    unsigned int startIndex;

    // Inner curve
    startIndex = vertices.size() / 3;
    nz = 0.0f;
    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = PI + i * sectorStep;    // starting from pi to 2*pi

        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);
        xIn = rInner * cosA;
        yIn = rInner * sinA;
        nx = -cosA;
        ny = -sinA;

        // front - back
        vertices.push_back(xIn);
        vertices.push_back(yIn);
        vertices.push_back(-length);

        vertices.push_back(xIn);
        vertices.push_back(yIn);
        vertices.push_back(length);

        normals.insert(normals.end(), { nx, ny, nz, nx, ny, nz });

        textureUVs.insert(textureUVs.end(), {
        (cosA * uvInner + 1.0f) * 0.5f, 0.0f,
        (sinA * uvInner + 1.0f) * 0.5f, 1.0f,
            });

        colors.insert(colors.end(), { 1.0f, 1.0f });
    }
    for (int i = 0; i < mSectors; ++i) {
        int k1 = i * 2 + startIndex;
        int k2 = k1 + 2;

        indices.push_back(k1);
        indices.push_back(k1 + 1);
        indices.push_back(k2);

        indices.push_back(k2);
        indices.push_back(k1 + 1);
        indices.push_back(k2 + 1);
    }


    // Outer curve
    startIndex = vertices.size() / 3;
    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = PI + i * sectorStep;
    
        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);
        xOut = rOuter * cosA;
        yOut = rOuter * sinA;
        nx = cosA;
        ny = sinA;

        // front - back
        vertices.push_back(xOut);
        vertices.push_back(yOut);
        vertices.push_back(-length);

        vertices.push_back(xOut);
        vertices.push_back(yOut);
        vertices.push_back(length);

        normals.insert(normals.end(), { nx, ny, nz, nx, ny, nz });

        textureUVs.insert(textureUVs.end(), {
        (cosA + 1.0f) * 0.5f, 0.0f,
        (sinA + 1.0f) * 0.5f, 1.0f,
            });

        colors.insert(colors.end(), { 1.0f, 1.0f });

    }
    for (int i = 0; i < mSectors; ++i) {
        int k1 = i * 2 + startIndex;
        int k2 = k1 + 2;

        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);

        indices.push_back(k2);
        indices.push_back(k2 + 1);
        indices.push_back(k1 + 1);
    }

    // Front face
    startIndex = vertices.size() / 3;
    nx = 0;
    ny = 0;
    nz = -1.0f;
    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = PI + i * sectorStep;

        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);
        xIn = rInner * cosA;
        yIn = rInner * sinA;
        xOut = rOuter * cosA;
        yOut = rOuter * sinA;

        // inner - outer
        vertices.push_back(xIn);
        vertices.push_back(yIn);
        vertices.push_back(-length);

        vertices.push_back(xOut);
        vertices.push_back(yOut);
        vertices.push_back(-length);

        normals.insert(normals.end(), { nx, ny, nz, nx, ny, nz });

        textureUVs.push_back((cosA * uvInner + 1.0f) * 0.5f);
        textureUVs.push_back((sinA * uvInner + 1.0f) * 0.5f);
        textureUVs.push_back((cosA + 1.0f) * 0.5f);
        textureUVs.push_back((sinA + 1.0f) * 0.5f);

        colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f });
    }
    for (int i = 0; i < mSectors; ++i) {
        int k1 = i * 2 + startIndex;
        int k2 = k1 + 2;

        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);

        indices.push_back(k2);
        indices.push_back(k2 + 1);
        indices.push_back(k1 + 1);
    }

    // Back face
    startIndex = vertices.size() / 3;
    nx = 0;
    ny = 0;
    nz = 1.0f;
    for (int i = 0; i <= mSectors; ++i) {
        float sectorAngle = PI + i * sectorStep;

        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);
        xIn = rInner * cosA;
        yIn = rInner * sinA;
        xOut = rOuter * cosA;
        yOut = rOuter * sinA;

        // inner - outer
        vertices.push_back(xIn);
        vertices.push_back(yIn);
        vertices.push_back(length);

        vertices.push_back(xOut);
        vertices.push_back(yOut);
        vertices.push_back(length);

        normals.insert(normals.end(), { nx, ny, nz, nx, ny, nz });

        textureUVs.push_back((cosA * uvInner + 1.0f) * 0.5f);
        textureUVs.push_back((sinA * uvInner + 1.0f) * 0.5f);
        textureUVs.push_back((cosA + 1.0f) * 0.5f);
        textureUVs.push_back((sinA + 1.0f) * 0.5f);

        colors.insert(colors.end(), { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f });
    }
    for (int i = 0; i < mSectors; ++i) {
        int k1 = i * 2 + startIndex;
        int k2 = k1 + 2;

        indices.push_back(k1);
        indices.push_back(k1 + 1);
        indices.push_back(k2);

        indices.push_back(k2);
        indices.push_back(k1 + 1);
        indices.push_back(k2 + 1);
    }

    // Squares
    startIndex = vertices.size() / 3;
    nx = 0;
    ny = 1.0f;
    nz = 0;

    vertices.insert(vertices.end(), { 
        -rOuter, 0.0f, -length,
        -rInner, 0.0f, -length,
        -rInner, 0.0f, length,
        -rOuter, 0.0f, length,

        rInner, 0.0f, -length,
        rOuter, 0.0f, -length,
        rOuter, 0.0f, length,
        rInner, 0.0f, length,
        });

    normals.insert(normals.end(), { 
        nx, ny, nz,
        nx, ny, nz,
        nx, ny, nz,
        nx, ny, nz,

        nx, ny, nz,
        nx, ny, nz,
        nx, ny, nz,
        nx, ny, nz,
        });

    textureUVs.insert(textureUVs.end(), {
        1.0f, 0.0f,
        uvInner, 0.0f,
        uvInner, 1.0f,
        1.0f, 1.0f,

        1.0f - uvInner, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f - uvInner, 1.0f,
        });

    indices.insert(indices.end(), {
        startIndex, startIndex + 3, startIndex + 2,
        startIndex, startIndex + 2, startIndex + 1,

        startIndex+4, startIndex + 7, startIndex + 6,
        startIndex+4, startIndex + 6, startIndex + 5,
        });

    glBindVertexArray(VAO);

    fillVertexBuffer(vertices);
    fillColorBuffer(colors);
    fillUVBuffer(textureUVs);
    fillNormalBuffer(normals);
    fillIndexBuffer(indices);

    mVertices = vertices;
    mIndices = indices;

    // Unbind VAO
    glBindVertexArray(0);
}
