/*==================================================================================
* Ray Tracer - Nicholas Coetzee
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "Cone.h"
#include "Cylinder.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 800;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;

//Only one of anti-aliasing and depth_of_field can be active,
// if both are set to true anti-aliasing takes priority
const bool anti_aliasing = false;

//Depth of field
const bool depth_of_field = true;
const float focus_dist = 80.0f;
const float aperture_radius = 0.8f;

vector<SceneObject*> sceneObjects;
TextureBMP texture;

glm::vec3 shadows(Ray ray, glm::vec3 color, glm::vec3 light1Pos, glm::vec3 light2Pos, SceneObject* obj) {
	//shadows
	glm::vec3 light1Vec = light1Pos - ray.hit;
	glm::vec3 light2Vec = light2Pos - ray.hit;
	Ray shadow1Ray(ray.hit, light1Vec);
	Ray shadow2Ray(ray.hit, light2Vec);
	shadow1Ray.closestPt(sceneObjects);
	shadow2Ray.closestPt(sceneObjects);

	bool shadow1 = false;
	bool shadow2 = false;

	//Reflective and Refractive objects cast lighter shadows
	bool lighterShadow1 = false;
	bool lighterShadow2 = false;

	if (shadow1Ray.index >= 0 && shadow1Ray.index < sceneObjects.size()) {
		SceneObject* shadowObj1 = sceneObjects[shadow1Ray.index];

		if (shadowObj1->isRefractive() || shadowObj1->isTransparent()) {
			lighterShadow1 = true;
		}
	}

	if (shadow2Ray.index >= 0 && shadow2Ray.index < sceneObjects.size()) {
		SceneObject* shadowObj2 = sceneObjects[shadow2Ray.index];

		if (shadowObj2->isRefractive() || shadowObj2->isTransparent()) {
			lighterShadow2 = true;
		}
	}

	if (shadow1Ray.index > -1 && shadow1Ray.dist < glm::length(light1Vec))
	{
		shadow1 = true;
	}
	if (shadow2Ray.index > -1 && shadow2Ray.dist < glm::length(light2Vec))
	{
		shadow2 = true;
	}

	if (shadow1 || shadow2)
	{
		if (lighterShadow1 && lighterShadow2) color = 0.8f * obj->getColor();
		else if (lighterShadow1 || lighterShadow2) color = 0.6f * obj->getColor();
		else color = 0.3f * obj->getColor();
	}
	if (shadow1 && shadow2)
	{
		if (lighterShadow1 && lighterShadow2) color = 0.5f * obj->getColor();
		else if (lighterShadow1 || lighterShadow2) color = 0.3f * obj->getColor();
		else color = 0.1f * obj->getColor();
	}

	return color;
}

glm::vec3 refract(const glm::vec3& I, const glm::vec3& N, float index1, float index2) {
    float index = index1 / index2;
    glm::vec3 n = normalize(N);
    glm::vec3 i = normalize(I);

    float cosI = glm::clamp(-glm::dot(n, i), -1.0f, 1.0f);

    if (cosI < 0)
    {
        cosI = -cosI;
        n = -n;  // negate normalized normal
        index = 1 / index;
    }

    float k = 1 - index * index * (1 - cosI * cosI);
    if (k < 0) return glm::vec3(0); //'Total internal reflection'
    else 
        return index * i + (index * cosI - sqrt(k)) * n;
}

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step) {
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 light1Pos(-20, 15, -10);					//Light's position
	glm::vec3 light2Pos(20, 15, -10);
	glm::vec3 color(0);
	SceneObject* obj;

	ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
	if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	//Plane
	if (ray.index == 0)
	{
		//Chequered pattern
		int zWidth = 4;
		int xWidth = 4;
		int iz = int(floor(ray.hit.z / zWidth));
		int ix = int(floor(ray.hit.x / xWidth));
		int k = (iz + ix) % 2;

		if (k == 0)
			color = glm::vec3(0, 1, 0);  // Green
		else
			color = glm::vec3(1, 0, 1);  // Magenta

		obj->setColor(color);

		//Texture mapping
		/*int x1 = -15;
		int x2 = 5;
		int z1 = -60;
		int z2 = -90;
		float texcoords = (ray.hit.x - x1)/(x2 - x1);
		float texcoordt = (ray.hit.z - z1)/(z2 - z1);
		if (texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1)
		{
			color = texture.getColorAt(texcoords, texcoordt);
			obj->setColor(color);
		}*/
	}

	//SceneObjects lighting calculations
	color = obj->lighting(light1Pos, -ray.dir, ray.hit) + obj->lighting(light2Pos, -ray.dir, ray.hit);

	//Custom Shadows (2 light sources)
	color = shadows(ray, color, light1Pos, light2Pos, obj);

	//Reflectivity
	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = (1.0f - rho) * color + rho * reflectedColor;
	}

	//Transparency and refractivity
	bool transparent = obj->isTransparent();
	bool refractive = obj->isRefractive();

	if ((transparent || refractive) && step < MAX_STEPS)
	{
		glm::vec3 transmittedColor;

		//Need to bend ray
		if (refractive)
		{
			float refractionCoeff = obj->getRefractionCoeff();
			float refractionIndex = obj->getRefractiveIndex();

			glm::vec3 normal = obj->normal(ray.hit);
			glm::vec3 refractedDir = refract(ray.dir, normal, 1.0f, refractionIndex);

			if (refractedDir != glm::vec3(0))
			{
				Ray refactedRay(ray.hit, refractedDir);
				transmittedColor = refractionCoeff * trace(refactedRay, step + 1);
			}

			//Light wasn't bent, treat as transparency
			else
			{
				Ray transmittedRay(ray.hit, ray.dir);
				transmittedColor = refractionCoeff * trace(transmittedRay, step + 1);
			}

			color = (1 - refractionCoeff) * color + transmittedColor;
		}

		//Only transparent
		else
		{
			float tranCoeff = obj->getTransparencyCoeff();
			Ray transmittedRay(ray.hit, ray.dir);
			transmittedColor = tranCoeff * trace(transmittedRay, step + 1);
			color = (1 - tranCoeff) * color + transmittedColor;
		}
	}

	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display() {
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.
	
	int samples = 4; //Samples per cell
	float invSamples = 1.0f / samples;

	for (int i = 0; i < NUMDIV; i++) {    // Scan every cell of the image plane
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++) {
			yp = YMIN + j * cellY;

			glm::vec3 col = glm::vec3(1);

			if (anti_aliasing)
			{
				glm::vec3 colorSum(0.0f);

				for (int sx = 0; sx < 2; sx++) {     // 2x2 grid within cell
					for (int sy = 0; sy < 2; sy++) {
						float sampleX = xp + (sx + 0.5f) * (cellX * 0.5f);
						float sampleY = yp + (sy + 0.5f) * (cellY * 0.5f);

						glm::vec3 dir(sampleX, sampleY, -EDIST);
						Ray ray(eye, dir);

						colorSum += trace(ray, 1);
					}
				}

				col = colorSum * 0.25f;  // Average color of 4 samples
			}

			else if (depth_of_field)
			{
				glm::vec3 colorSum(0.0f);
				int numSamples = 4;

				for (int s = 0; s < numSamples; s++)
				{
					// Sample point (center of pixel + randomness)
					float sampleX = xp + cellX * ((float)rand() / RAND_MAX);
					float sampleY = yp + cellY * ((float)rand() / RAND_MAX);
					glm::vec3 pixelPoint(sampleX, sampleY, -EDIST);

					// Compute focal point
					glm::vec3 dir = glm::normalize(pixelPoint - eye);
					glm::vec3 focalPoint = eye + dir * focus_dist;

					//Sample a random point on aperture disk (lens)
					float r = aperture_radius * sqrt((float)rand() / RAND_MAX);
					float theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);

					float dx = r * cos(theta);
					float dy = r * sin(theta);

					glm::vec3 lensOrigin = eye + glm::vec3(dx, dy, 0.0f); // lens in xy plane
					glm::vec3 dofDir = glm::normalize(focalPoint - lensOrigin);

					//Trace
					Ray ray(lensOrigin, dofDir);
					colorSum += trace(ray, 1);
				}

				col = colorSum * (1.0f / numSamples); // Average
			}

			else
			{
				glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

				Ray ray = Ray(eye, dir);

				col = trace(ray, 1); //Trace the primary ray and get the colour value
			}


			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);             // Draw each cell with averaged color
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
}

void DrawWalls(void) {
	//floor
	Plane *plane = new Plane (glm::vec3(-30., -15, -40), //Point A
							glm::vec3(30., -15, -40), //Point B
							glm::vec3(30., -15, -150), //Point C
							glm::vec3(-30., -15, -150)); //Point D
	plane->setSpecularity(false);

	//roof
	Plane *plane2 = new Plane(glm::vec3(-30., 15, -40),
                          glm::vec3(-30., 15, -150),
                          glm::vec3(30., 15, -150),
                          glm::vec3(30., 15, -40));
	plane2->setColor(glm::vec3(0, 0, 1));
	plane2->setSpecularity(false);
	
	Plane *plane3 = new Plane(glm::vec3(-30., -15, -40),
                          glm::vec3(-30., -15, -150),
                          glm::vec3(-30., 15, -150),
                          glm::vec3(-30., 15, -40));
	plane3->setColor(glm::vec3(1, 0, 0));
	plane3->setSpecularity(false);

	Plane *plane4 = new Plane(glm::vec3(30., -15, -150),
                          glm::vec3(30., -15, -40),
                          glm::vec3(30., 15, -40),
                          glm::vec3(30., 15, -150));
	plane4->setColor(glm::vec3(0, 1, 0));
	plane4->setSpecularity(false);

	Plane *plane5 = new Plane(glm::vec3(-30., -15, -150),
                          glm::vec3(30., -15, -150),
                          glm::vec3(30., 15, -150),
                          glm::vec3(-30., 15, -150));
	plane5->setColor(glm::vec3(0, 1, 1));
	plane5->setSpecularity(false);

	sceneObjects.push_back(plane);
	sceneObjects.push_back(plane2);
	sceneObjects.push_back(plane3);
	sceneObjects.push_back(plane4);
	sceneObjects.push_back(plane5);
}

void DrawObjects(void) {
	Plane *mirror = new Plane(
							glm::vec3(-10., -5, -100),
							glm::vec3(10., -5, -100),
							glm::vec3(10., 5, -95),
							glm::vec3(-10., 5, -95)
							);
	mirror->setColor(glm::vec3(1, 1, 1));
	mirror->setReflectivity(true, 1);
	mirror->setSpecularity(false);

	Sphere *sphere1 = new Sphere(glm::vec3(-5.0, -8.0, -90.0), 5.0);
	sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
	sphere1->setSpecularity(true);
	sphere1->setTransparency(true, 0.6);

	Sphere *sphere2 = new Sphere(glm::vec3(5.0, 5.0, -70.0), 4.0);
	sphere2->setColor(glm::vec3(0, 1, 0));
	sphere2->setShininess(5);

	Sphere *sphere3 = new Sphere(glm::vec3(10.0, 10.0, -60.0), 3.0);
	sphere3->setColor(glm::vec3(1, 0, 0));

	Sphere *sphere4 = new Sphere(glm::vec3(5.0, -10.0, -60.0), 5.0);
	sphere4->setColor(glm::vec3(0.5, 1, 1));
	sphere4->setTransparency(true, 0.8f);

	Sphere *sphere5 = new Sphere(glm::vec3(-5, -7, -50.0), 4.0);
	sphere5->setColor(glm::vec3(0.8, 0, 0.8));
	sphere5->setRefractivity(true, 0.7f, 1.2f);

	sceneObjects.push_back(mirror);
	sceneObjects.push_back(sphere1);
	//sceneObjects.push_back(sphere2);
	//sceneObjects.push_back(sphere3);
	//sceneObjects.push_back(sphere4);
	//sceneObjects.push_back(sphere5);

	Cone *cone = new Cone(glm::vec3(10, -10, -80), 8.f, 3.f);
	cone->setColor(glm::vec3(0.8, 0.8, 0));
	cone->setRefractivity(true, 0.7f, 1.2f);
	sceneObjects.push_back(cone);

	Cylinder* cyl = new Cylinder(glm::vec3(15, -15, -80), 5.0f, 1.0f);
	cyl->setColor(glm::vec3(0.8, 0.3, 0.3));
	sceneObjects.push_back(cyl);

}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize() {
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

	glClearColor(0, 0, 0, 1);

	texture = TextureBMP("../Butterfly.bmp");

	DrawWalls();

	DrawObjects();
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(40, 40);
	glutCreateWindow("Assignment 2 - nco63");

	glutDisplayFunc(display);
	initialize();

	glutMainLoop();
	return 0;
}