#include "Cone.h"

float Cone::intersect(glm::vec3 rayOrigin, glm::vec3 rayDir) {
    float rOverH = radius / height;
    float rOverH2 = rOverH * rOverH;

    // Ray direction components
    float dx = rayDir.x;
    float dy = rayDir.y;
    float dz = rayDir.z;

    // Vector from rayOrigin to apex
    glm::vec3 diff = rayOrigin - apex;
    float ox = diff.x;
    float oy = diff.y;
    float oz = diff.z;

    // Compute quadratic coefficients
    float A = dx*dx + dz*dz - rOverH2 * dy*dy;
    float B = 2.0f * (ox*dx + oz*dz - rOverH2 * oy*dy);
    float C = ox*ox + oz*oz - rOverH2 * oy*oy;

    // Compute discriminant
    float discriminant = B*B - 4*A*C;
    if (discriminant < 0.001f) return -1.0f;  // No real roots or tangent ray

    // Solve quadratic
    float sqrtDisc = sqrt(discriminant);
    float t1 = (-B - sqrtDisc) / (2*A);
    float t2 = (-B + sqrtDisc) / (2*A);

    // Choose smallest positive t
    float t = (t1 > 0.001f) ? t1 : ((t2 > 0.001f) ? t2 : -1.0f);
    if (t < 0.001f) return -1.0f;

    // Compute Y-coordinate of hit point
    glm::vec3 hitPoint = rayOrigin + t * rayDir;
    float hitY = hitPoint.y;

    // Check vertical bounds
    float minY = apex.y;
    float maxY = apex.y + height;
    if (hitY < minY || hitY > maxY)
        return -1.0f;

    return t;
}

glm::vec3 Cone::normal(glm::vec3 point) {
    glm::vec3 v = point - apex;
    glm::vec2 xz = glm::vec2(v.x, v.z);      // Project onto xz-plane

    float slope = radius / height;           // Cone slope = radius / height

    // Surface normal vector
    glm::vec3 normal = glm::normalize(glm::vec3(
        xz.x,
        slope * glm::length(xz),  // Y component scaled by cone slope
        xz.y
    ));

    return normal;
}