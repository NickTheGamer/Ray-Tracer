/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#ifndef H_SPHERE
#define H_SPHERE
#include <glm/glm.hpp>
#include "SceneObject.h"

/**
 * Defines a simple Sphere located at 'center'
 * with the specified radius
 */
class Sphere : public SceneObject {
private:
	glm::vec3 center = glm::vec3(0);
	float radius = 1;

	//Transformation matrices
	glm::mat4 transform = glm::mat4(1.0f);           // World-to-object
	glm::mat4 inverseTransform = glm::mat4(1.0f);    // Object-to-world


public:
	Sphere() {};  //Default constructor creates a unit sphere

	Sphere(glm::vec3 c, float r) : center(c), radius(r) {}

	float intersect(glm::vec3 p0, glm::vec3 dir);

	glm::vec3 normal(glm::vec3 p);

	//Used for texturing
	glm::vec3 getCenter();

	//Used for object transformations e.g. flattening
	void setTransform(glm::vec3 transform);
};

#endif //!H_SPHERE
