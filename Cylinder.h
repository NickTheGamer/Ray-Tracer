#ifndef CYLINDER_H
#define CYLINDER_H

#include <glm/glm.hpp>
#include "SceneObject.h"

/**
 * Defines a vertical cylinder (Y-up) with:
 * - a center at the base (x, y, z)
 * - modifiable height and radius
 * - visible end caps
 */
class Cylinder : public SceneObject {
private:
    glm::vec3 baseCenter = glm::vec3(0);  // Center of the bottom cap
    float radius = 1.0f;
    float height = 1.0f;

public:
    Cylinder() = default;

    Cylinder(glm::vec3 base, float h, float r) : baseCenter(base), height(h), radius(r) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};

#endif // CYLINDER_H