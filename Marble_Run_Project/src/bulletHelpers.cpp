#include "bulletHelpers.h"

btQuaternion quatFromYawPitchRoll(btScalar yaw, btScalar pitch, btScalar roll)
{
    // Input in degrees
    // Order: yaw (Z), pitch (Y), roll (X)

    btQuaternion q;
    q.setEulerZYX(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));

    return q;
}

btTriangleMesh* createBtTriangleMesh(Shape* shape)
{
    std::vector<float> vertices = shape->mVertices;
    std::vector<unsigned int> indices = shape->mIndices;

    if (vertices.size() == 0) std::cout << "Shape has 0 vertices saved!" << std::endl;

    btTriangleMesh* mesh = new btTriangleMesh();

    for (int i = 0; i < indices.size(); i += 3)
    {
        int i0 = indices[i];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];

        btVector3 v0(vertices[3 * i0], vertices[3 * i0 + 1], vertices[3 * i0 + 2]);
        btVector3 v1(vertices[3 * i1], vertices[3 * i1 + 1], vertices[3 * i1 + 2]);
        btVector3 v2(vertices[3 * i2], vertices[3 * i2 + 1], vertices[3 * i2 + 2]);

        mesh->addTriangle(v0, v1, v2);
    }

    return mesh;
}

btRigidBody* createMarbleRigidBody(btScalar mass, btScalar radius, btVector3 origin, btScalar rest, btScalar fric)
{
    btCollisionShape* shape = new btSphereShape(radius);

    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(mass, inertia);

    btQuaternion q = btQuaternion(0, 0, 0, 1);

    btDefaultMotionState* motionState =
        new btDefaultMotionState(btTransform(q, origin));

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
        mass,
        motionState,
        shape,
        inertia);

    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setRestitution(rest);
    rigidBody->setFriction(fric);
    rigidBody->setActivationState(DISABLE_DEACTIVATION);
    //rigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, -3.0f)); // temp

    return rigidBody;
}

btRigidBody* createStaticRigidBody(btTriangleMesh* mesh, btVector3 origin, btQuaternion rotation, btScalar rest, btScalar fric)
{
    bool useQuantizedAabbCompression = true; // usually true for performance
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(mesh, useQuantizedAabbCompression);

    btDefaultMotionState* motionState = 
        new btDefaultMotionState(btTransform(rotation, origin));

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
        0,
        motionState,
        shape,
        btVector3(0, 0, 0));

    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setRestitution(rest);
    rigidBody->setFriction(fric);

    return rigidBody;
}

btRigidBody* createStaticRigidBody(btCollisionShape* shape, btVector3 origin, btQuaternion rotation, btScalar rest, btScalar fric)
{
    btDefaultMotionState* motionState =
        new btDefaultMotionState(btTransform(rotation, origin));

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
        0,
        motionState,
        shape,
        btVector3(0, 0, 0));

    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setRestitution(rest);
    rigidBody->setFriction(fric);

    return rigidBody;
}


