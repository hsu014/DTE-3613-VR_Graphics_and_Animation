#include "shape.h"

TestShape::TestShape() 
{
    fillBuffers();
}

TestShape::~TestShape()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void TestShape::fillBuffers()
{
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  1.0f, 0.4f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  1.0f, 0.7f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,  1.0f, 0.4f, 0.0f,  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);
}

void TestShape::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}



Box::Box()
{
    fillBuffers();
}

Box::~Box()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(2, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
};

void Box::fillBuffers()
{
    float vertices[] = {
        // Front face
        -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f,  1.0f, -1.0f,   1.0f, 0.7f, 0.0f,
        -1.0f,  1.0f,  1.0f,   1.0f, 0.4f, 0.0f,

        // Back face
         1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   1.0f, 0.4f, 0.0f,
         1.0f,  1.0f,  1.0f,   1.0f, 0.7f, 0.0f,
         1.0f,  1.0f, -1.0f,   1.0f, 0.4f, 0.0f,

        // Right face
         1.0f, -1.0f,  1.0f,   1.0f, 0.4f, 0.0f,
        -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,   1.0f, 0.4f, 0.0f,
         1.0f,  1.0f,  1.0f,   1.0f, 0.7f, 0.0f,

        // Left face
        -1.0f, -1.0f, -1.0f,  1.0f, 0.4f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 0.4f, 0.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 0.7f, 0.0f,

        // Top face
        -1.0f,  1.0f,  1.0f,  1.0f, 0.4f, 0.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 0.7f, 0.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 0.4f, 0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 0.7f, 0.0f,

        // Bottom face
         1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 0.4f, 0.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  1.0f, 0.4f, 0.0f,
    };

    float normals[] = {
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

    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        // Back face
        4, 5, 6,
        6, 7, 4,
        // Right face
        8, 9, 10,
        10, 11, 8,
        // Left face
        12, 13, 14,
        14, 15, 12,
        // Top face
        16, 17, 18,
        18, 19, 16,
        // Bottom face
        20, 21, 22,
        22, 23, 20,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind VAO
    glBindVertexArray(0);

};

void Box::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}



Pyramid::Pyramid()
{
    fillBuffers();
}

Pyramid::~Pyramid()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Pyramid::fillBuffers()
{
    float vertices[] = {
        // Side 1
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 0.0f,
         0.0f,  1.0f,  0.0f,   1.0f, 0.0f, 0.0f,

        // Side 2
         1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,
         0.0f,  1.0f,  0.0f,   1.0f, 0.0f, 0.0f,

        // Side 3
         1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,    1.0f, 0.0f, 0.0f,

        // Side 4 
        -1.0f, -1.0f,  1.0f,   0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, 1.0f,
         0.0f,  1.0f,  0.0f,   1.0f, 0.0f, 0.0f,

        // Bottom    
         1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, 1.0f,   
    };

    unsigned int indices[] = {
        // Sides
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        // Bottom
        12, 13, 14,
        14, 15, 12,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);
}

void Pyramid::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}



CompositePlane::CompositePlane(int width, int depth, GLuint texture)
    : mWidth(width), mDepth(depth), mTexture(texture), mHeightMap(nullptr)
{
    fillBuffers();
}

CompositePlane::CompositePlane(
    GLuint texture,
    std::shared_ptr<std::vector<std::vector<float>>> heightMap)
    : mTexture(texture), mHeightMap(heightMap)
{
    if (mHeightMap && !mHeightMap->empty() && !(*mHeightMap)[0].empty()) {
        mWidth = (*mHeightMap).size();
        mDepth = (*mHeightMap)[0].size();
    }
    else {
        std::cout << "Heightmap is empty or not assigned.\n";
    }

    fillBuffers();
}

CompositePlane::~CompositePlane()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(numVBOs, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void CompositePlane::fillBuffers()
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> textureUVs;

    std::vector<unsigned int> indices;

    float x_to_z_ratio = static_cast<float>(mDepth) / mWidth;
    float scale = 0.1f;


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
        }
    }

    for (int x = 0; x < mWidth -1; x++) {
        for (int z = 0; z < mDepth -1; z++) {

            int start = x * mDepth + z;

            indices.push_back(start);
            indices.push_back(start + mDepth);
            indices.push_back(start + 1);

            indices.push_back(start + 1);
            indices.push_back(start + mDepth);
            indices.push_back(start + mDepth + 1);
        }
    }


    indexCount = static_cast<GLsizei>(indices.size());
    //std::cout << "vertexCount: " << vertices.size() << std::endl;
    //std::cout << "indexCount:  " << indexCount << std::endl;

    //if (mTexture) std::cout << "Texture found" << std::endl;
    //else std::cout << "No texture found" << std::endl;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(numVBOs, VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // texture UV attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, textureUVs.size() * sizeof(float), &textureUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    // index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    if (mTexture)
    {
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void CompositePlane::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}



Sphere::Sphere(int sectors = 10, int stacks = 10) : mSectors(sectors), mStacks(stacks)
{
    fillBuffers();
}

Sphere::~Sphere()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(numVBOs , VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Sphere::fillBuffers()
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> textureUVs;
    std::vector<float> normals;

    std::vector<unsigned int> indices;

    const float PI = acos(-1.0f);
    const float radius = 1;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float u, v;                                     // texCoord
    float cr = 1.0f, cg = 1.0f, cb = 1.0f;          // color

    float sectorStep = 2 * PI / mSectors;
    float stackStep = PI / mStacks;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= mStacks; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= mSectors; ++j)
        {
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
            colors.push_back(cr);
            colors.push_back(cg);
            colors.push_back(cb);
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < mStacks; ++i)
    {
        k1 = i * (mSectors + 1);     // beginning of current stack
        k2 = k1 + mSectors + 1;      // beginning of next stack

        for (int j = 0; j < mSectors; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if (i != 0)
            {
                //addIndices(k1, k2, k1 + 1);   // k1---k2---k1+1
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (mStacks - 1))
            {
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

    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(numVBOs, VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // texture UV attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, textureUVs.size() * sizeof(float), &textureUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    // normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);

    // index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    if (mTexture)
    {
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);
    }

    // Unbind VAO
    glBindVertexArray(0);
    

}

void Sphere::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}
