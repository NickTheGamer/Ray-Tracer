/*==================================================================================
* A basic ray tracer
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
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 800;
const int MAX_STEPS = 10;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;

vector<SceneObject*> sceneObjects;
TextureBMP texture;

glm::vec3 Shadows(Ray ray, glm::vec3 color, glm::vec3 light1Pos, glm::vec3 light2Pos, SceneObject* obj) {
	//shadows
	glm::vec3 light1Vec = light1Pos - ray.hit;
	glm::vec3 light2Vec = light2Pos - ray.hit;
	Ray shadow1Ray(ray.hit, light1Vec);
	Ray shadow2Ray(ray.hit, light2Vec);
	shadow1Ray.closestPt(sceneObjects);
	shadow2Ray.closestPt(sceneObjects);

	bool shadow1 = false;
	bool shadow2 = false;
	bool lighterShadow1 = false;
	bool lighterShadow2 = false;

	if (shadow1Ray.index >= 0 && shadow1Ray.index < sceneObjects.size()) {
		SceneObject* shadowObj1 = sceneObjects[shadow1Ray.index];

		if (shadowObj1->isRefractive() || shadowObj1->isReflective()) {
			lighterShadow1 = true;
		}
	}

	if (shadow2Ray.index >= 0 && shadow2Ray.index < sceneObjects.size()) {
		SceneObject* shadowObj2 = sceneObjects[shadow2Ray.index];

		if (shadowObj2->isRefractive() || shadowObj2->isReflective()) {
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

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step) {
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 light1Pos(20, 40, -3);					//Light's position
	glm::vec3 light2Pos(-20, 40, -3);
	glm::vec3 color(0);
	SceneObject* obj;

	ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
	if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	//Plane
	if (ray.index == 4)
	{
		//Stripes
		int stripeWidth = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int k = iz % 2;
		if (k == 0) color = glm::vec3(0, 1, 0);
		else color = glm::vec3(1, 1, 0.5);
		obj->setColor(color);

		//Texture mapping
		int x1 = -15;
		int x2 = 5;
		int z1 = -60;
		int z2 = -90;
		float texcoords = (ray.hit.x - x1)/(x2 - x1);
		float texcoordt = (ray.hit.z - z1)/(z2 - z1);
		if (texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1)
		{
			color = texture.getColorAt(texcoords, texcoordt);
			obj->setColor(color);
		}
	}

	//SceneObjects lighting calculations
	color = obj->lighting(light1Pos, -ray.dir, ray.hit) + obj->lighting(light2Pos, -ray.dir, ray.hit);

	//Custom Shadows (2 light sources)
	color = Shadows(ray, color, light1Pos, light2Pos, obj);

	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
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

	for (int i = 0; i < NUMDIV; i++) {	//Scan every cell of the image plane
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++) {
			yp = YMIN + j * cellY;

			glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

			Ray ray = Ray(eye, dir);

			glm::vec3 col = trace(ray, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
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

	Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -90.0), 15.0);
	sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
	sphere1->setSpecularity(false);
	sphere1->setReflectivity(true, 0.8);

	Sphere *sphere2 = new Sphere(glm::vec3(5.0, 5.0, -70.0), 4.0);
	sphere2->setColor(glm::vec3(0, 1, 0));
	sphere2->setShininess(5);

	Sphere *sphere3 = new Sphere(glm::vec3(10.0, 10.0, -60.0), 3.0);
	sphere3->setColor(glm::vec3(1, 0, 0));

	Sphere *sphere4 = new Sphere(glm::vec3(5.0, -10.0, -60.0), 5.0);
	sphere4->setColor(glm::vec3(0.5, 1, 1));

	Plane *plane = new Plane (glm::vec3(-20., -15, -40), //Point A
							  glm::vec3(20., -15, -40), //Point B
							  glm::vec3(20., -15, -200), //Point C
							  glm::vec3(-20., -15, -200)); //Point D
	plane->setColor(glm::vec3(0.8, 0.8, 0));
	plane->setSpecularity(false);

	sceneObjects.push_back(sphere1);
	sceneObjects.push_back(sphere2);
	sceneObjects.push_back(sphere3);
	sceneObjects.push_back(sphere4);
	sceneObjects.push_back(plane);
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