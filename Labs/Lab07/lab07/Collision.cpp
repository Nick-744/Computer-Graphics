#include "Collision.h"
#include "Box.h"
#include "Sphere.h"
using namespace glm;

void handleBoxSphereCollision(Box& box, Sphere& sphere);
bool checkForBoxSphereCollision(glm::vec3 &pos, const float& r, const float& size, glm::vec3& n);

// ===< HOMEWORK 2 >=== //
void handleSphereSphereCollision(Sphere& sphere1, Sphere& sphere2);
bool checkForSphereSphereCollision(glm::vec3& pos1, const float& r1, glm::vec3& pos2, const float& r2, glm::vec3& n);

void handleBoxSphereCollision(Box& box, Sphere& sphere)
{
    vec3 n;
    if (checkForBoxSphereCollision(sphere.x, sphere.r, box.size, n))
    {
        // Task 2b: define the velocity of the sphere after the collision
		//sphere.v = reflect(sphere.v, normalize(sphere.v * n)); // My Solution!
		sphere.v = sphere.v - 2.0f * dot(sphere.v, n) * n;
		sphere.P = sphere.m * sphere.v; // Ορμή!
    }
}

bool checkForBoxSphereCollision(vec3 &pos, const float& r, const float& size, vec3& n)
{
    if (pos.x - r <= 0)
    {
        //correction
        float dis = -(pos.x - r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(-1, 0, 0);
    }
    else if (pos.x + r >= size)
    {
        //correction
        float dis = size - (pos.x + r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(1, 0, 0);
    }
    else if (pos.y - r <= 0)
    {
        //correction
        float dis = -(pos.y - r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, -1, 0);
    }
    else if (pos.y + r >= size)
    {
        //correction
        float dis = size - (pos.y + r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, 1, 0);
    }
    else if (pos.z - r <= 0)
    {
        //correction
        float dis = -(pos.z - r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, -1);
    }
    else if (pos.z + r >= size)
    {
        //correction
        float dis = size - (pos.z + r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, 1);
    }
    else return false;

    return true;
}



// ===< HOMEWORK 2 >=== //
void handleSphereSphereCollision(Sphere& sphere1, Sphere& sphere2)
{
    vec3 n;
    if (checkForSphereSphereCollision(sphere1.x, sphere1.r, sphere2.x, sphere2.r, n))
    {
        vec3 v1i = sphere1.v;
        vec3 v2i = sphere2.v;

        float m1 = sphere1.m;
        float m2 = sphere2.m;

        sphere1.v = v1i - (2.0f * m2 / (m1 + m2)) * dot(v1i - v2i, n)  *  n;
        sphere2.v = v2i - (2.0f * m1 / (m1 + m2)) * dot(v2i - v1i, -n) * -n;

        // Ορμή!
        sphere1.P = sphere1.m * sphere1.v;
		sphere2.P = sphere2.m * sphere2.v;
	}
}

bool checkForSphereSphereCollision(glm::vec3& pos1, const float& r1, glm::vec3& pos2, const float& r2, glm::vec3& n)
{
    vec3 dir        = pos2 - pos1;
    float distSq    = dot(dir, dir);
    float radiusSum = r1 + r2;

    if (distSq <= radiusSum * radiusSum)
    {
        n = normalize(dir);
        return true;
    }

	return false;
}
