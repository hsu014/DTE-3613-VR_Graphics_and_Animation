#include "shape.h"

TestShape::TestShape() 
{
    fillBuffers();
}

TestShape::~TestShape()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void TestShape::fillBuffers()
{
    std::cout << "Fill buffer" << std::endl;
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
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
}



Box::Box()
{
    fillBuffers();
}

Box::~Box()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
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
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

};

void Box::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

Pyramid::Pyramid()
{
    fillBuffers();
}

Pyramid::~Pyramid()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
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
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

}
