#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>

class Box;
class Sphere;
void handleBoxSphereCollision(Box& box, Sphere& sphere);

// ===< HOMEWORK 2 >=== //
void handleSphereSphereCollision(Sphere& sphere1, Sphere& sphere2);

#endif
