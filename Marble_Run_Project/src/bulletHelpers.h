#pragma once

#include <vector>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>
#include "shape.h"

btQuaternion quatFromYawPitchRoll(btScalar yaw = 0.0f, btScalar pitch = 0.0f, btScalar roll = 0.0f);

btTriangleMesh* createBtTriangleMesh(Shape* shape);

btRigidBody* createMarbleRigidBody(
	btScalar mass = 1.0f, btScalar radius = 0.1f, btVector3 origin = btVector3(0.0, 0.0, 0.0), 
	btScalar rest = 0.5f, btScalar fric = 0.8f);

btRigidBody* createStaticRigidBody(
	btTriangleMesh* mesh, btVector3 origin = btVector3(0.0, 0.0, 0.0), 
	btQuaternion rotation = btQuaternion(0, 0, 0, 1), btScalar rest = 0.5f, btScalar fric = 0.8f);

btRigidBody* createStaticRigidBody(
	btCollisionShape* shape, btVector3 origin = btVector3(0.0, 0.0, 0.0),
	btQuaternion rotation = btQuaternion(0, 0, 0, 1), btScalar rest = 0.5f, btScalar fric = 0.8f);

