
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <Magick++.h>
#include "ImageBuffer.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

using namespace std;

//TODO, make window height and width a property of the scene, base the normalization off of this
class Scene
{
	private:
	    ImageBuffer image;
	    void draw();
	    void commit();
}

class Tracer
{
	std::vector<Light*> lights;
	std::vector<Object*> objects;
	void trace(Ray &ray, glm::vec3 *colour, GLuint recursion_depth);
	~Tracer();
}

class Camera
{
	public:
	    Camera(GLfloat fov_);
	    void generate_ray(GLuint x, GLuint y, Ray *ray);
	    GLfloat fov;
	private:
	    glm::vec2 normalize_pixel(GLuint x, GLuint y);
}

class Ray
{
	Ray(glm::vec3 origin_, glm::vec3 direction_);
	glm::vec3 origin;
	glm::vec3 direction;
}

class Light
{
	public:
	    Light(glm::vec3 point_);
	    void generate_light_ray(glm::vec3 &scene_intersection, Ray *lray, glm::vec3 *lcolour);
	private:
        glm::vec3 point;
}

//Primitives

class Object
{
	glm::vec3 colour;
	glm::vec3 normal;
	virtual bool intersect(Ray &ray) = 0;
}

class Sphere : public Object
{
	public:
	    Sphere(glm::vec3 center_, GLfloat radius_);
	    bool intersect(Ray &ray);
	private:
		glm::vec3 center;
	    GLfloat radius;
}

class Plane : public Object
{
	public:
	    Plane(glm::vec3 normal_, glm::vec3 point_);
	    bool intersect(Ray &ray);
	private:
		glm::vec3 normal;
	    glm::vec3 point;
}

class Triangle : public Object
{
	public:
	    Triangle(glm::vec3 p0_, glm::vec3 p1_, glm::vec3 p2_);
		Triangle& operator=(const Triangle& other)
	    bool intersect(Ray &ray);
	private:
		glm::vec3 p0, p1, p2;
}

class Pyramid : public Triangle
{
	public:
	    Pyramid(Triangle t0_, Triangle t1_, Triangle t2_, Triangle t3_);
	    bool intersect(Ray &ray);
	private:
		Triangle t0, t1, t2, t3;
}

class Rectangle : public Triangle
{
	public:
	    Rectangle(Triangle t0_, Triangle t1_);
	    bool intersect(Ray &ray);
	private:
		Triangle t0, t1;
}