#ifndef CONE_H
#define CONE_H

#include <glm/glm.hpp>
#include "SceneObject.h"

/**
 * Defines a cone pointing upwards with:
 * - an apex at a specified x, y, z position
 * - modifiable base radius and height
 */
class Cone : public SceneObject {
private:
    glm::vec3 apex = glm::vec3(0);  // Apex
    float radius = 1.0f;
    float height = 1.0f;

public:
    Cone() = default;

    Cone(glm::vec3 a, float h, float r) : apex(a), radius(r), height(h) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};

#endif // CONE_H