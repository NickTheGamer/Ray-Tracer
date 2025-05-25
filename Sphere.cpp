/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Sphere.h"
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>

/**
* Sphere's intersection method.  The input is a ray. 
*/
/*float Sphere::intersect(glm::vec3 p0, glm::vec3 dir) {
	glm::vec3 vdif = p0 - center;   //Vector s (see Slide 28)
	float b = glm::dot(dir, vdif);
	float len = glm::length(vdif);
	float c = len*len - radius*radius;
	float delta = b*b - c;

	if(delta < 0.001) return -1.0;    //includes zero and negative values

	float t1 = -b - sqrt(delta);
	float t2 = -b + sqrt(delta);

	if (t1 < 0)
	{
		return (t2 > 0) ? t2 : -1;
	}
	else return t1;
}*/
float Sphere::intersect(glm::vec3 p0, glm::vec3 dir) {
    // World to object transformation
    glm::vec3 p0_obj = glm::vec3(inverseTransform * glm::vec4(p0, 1.0f));
    glm::vec3 dir_obj = glm::normalize(glm::vec3(inverseTransform * glm::vec4(dir, 0.0f)));

    // Sphere assumed centered at origin in object space
    glm::vec3 vdif = p0_obj - center;
    float b = glm::dot(dir_obj, vdif);
    float len = glm::length(vdif);
    float c = len * len - radius * radius;
    float delta = b * b - c;

    if (delta < 0.001f) return -1.0f;

    float t1 = -b - sqrt(delta);
    float t2 = -b + sqrt(delta);

    float t_obj = (t1 >= 0) ? t1 : (t2 >= 0 ? t2 : -1.0f);
    if (t_obj < 0) return -1.0f;

	// Object to world transformation
    glm::vec3 hit_obj = p0_obj + t_obj * dir_obj;
    glm::vec3 hit_world = glm::vec3(transform * glm::vec4(hit_obj, 1.0f));

    // Return distance from p0 (t value)
    return glm::length(hit_world - p0);
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Sphere::normal(glm::vec3 p) {
	// Transform into object space
	glm::vec3 p_obj = glm::vec3(inverseTransform * glm::vec4(p, 1.0));
	glm::vec3 n_obj = glm::normalize(p_obj - center);

	// Return normal in world space with inverse transpose
	glm::vec3 normal = glm::normalize(glm::vec3(glm::transpose(inverseTransform) * glm::vec4(n_obj, 0.0)));
	return normal;
}

glm::vec3 Sphere::getCenter()
{
	return center;
}

void Sphere::setTransform(glm::vec3 vector)
{
	transform = glm::scale(glm::mat4(1.0f), vector);
	inverseTransform = glm::inverse(transform);
}
