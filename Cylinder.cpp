#include "Cylinder.h"
#include <glm/glm.hpp>
#include <cmath>

//For max float value as placeholder t value
#include <limits>

//For iterations through possible t values
#include <initializer_list>

float Cylinder::intersect(glm::vec3 rayOrigin, glm::vec3 rayDir) {
    glm::vec3 p = rayOrigin - baseCenter;

    //Calculate coefficients
    float a = rayDir.x * rayDir.x + rayDir.z * rayDir.z;
    float b = 2.0f * (p.x * rayDir.x + p.z * rayDir.z);
    float c = p.x * p.x + p.z * p.z - radius * radius;

    //Calculate number of intersections with the cylinders' sides
    float discriminant = b * b - 4.0f * a * c;
    float tCylinder = -1.0f;

    //Get smallest t value on side of cylinder
    if (discriminant >= 0.0f) {
        float sqrtDisc = sqrt(discriminant);
        float t1 = (-b - sqrtDisc) / (2.0f * a);
        float t2 = (-b + sqrtDisc) / (2.0f * a);

        for (float t : {t1, t2}) {
            float y = p.y + t * rayDir.y;
            if (t > 0 && y >= 0.0f && y <= height) {
                if (tCylinder < 0 || t < tCylinder) {
                    tCylinder = t;
                }
            }
        }
    }

    // Check bottom
    float tBottom = -1.0f;
    if (std::abs(rayDir.y) > 1e-4f) {
        float t = -p.y / rayDir.y;
        glm::vec3 hit = p + t * rayDir;
        if (t > 0 && hit.x * hit.x + hit.z * hit.z <= radius * radius) {
            tBottom = t;
        }
    }

    // Check top
    float tTop = -1.0f;
    if (std::abs(rayDir.y) > 1e-4f) {
        float t = (height - p.y) / rayDir.y;
        glm::vec3 hit = p + t * rayDir;
        if (t > 0 && hit.x * hit.x + hit.z * hit.z <= radius * radius) {
            tTop = t;
        }
    }

    // Choose the smallest positive t between the side, top and bottom
    float tFinal = std::numeric_limits<float>::max();
    for (float t : {tCylinder, tBottom, tTop}) {
        if (t > 0 && t < tFinal) {
            tFinal = t;
        }
    }

    //Return t or -1 if placeholder not replaced
    return (tFinal < std::numeric_limits<float>::max()) ? tFinal : -1.0f;
}

glm::vec3 Cylinder::normal(glm::vec3 p) {
    glm::vec3 localP = p - baseCenter;
    float y = localP.y;

    if (std::abs(y) < 0.001f) return glm::vec3(0, -1, 0); // bottom cap
    if (std::abs(y - height) < 0.001f) return glm::vec3(0, 1, 0); // top cap

    // Side normal
    glm::vec3 n(localP.x, 0, localP.z);
    return glm::normalize(n);
}