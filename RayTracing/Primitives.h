#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Ray.h"
using namespace glm;

class Object
{
    public:
	    vec3 colour;
	    vec3 normal;
	    virtual bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val) = 0;
};

class Sphere : public Object
{
	public:
	    Sphere(vec3 center_, GLfloat radius_);
	    bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val);
	private:
		vec3 center;
	    GLfloat radius;
};

class Plane : public Object
{
	public:
	    Plane(vec3 normal_, vec3 point_);
	    bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val);
	private:
		vec3 normal;
	    vec3 point;
};

class Triangle : public Object
{
	public:
	    Triangle(vec3 p0_, vec3 p1_, vec3 p2_);
		Triangle& operator=(const Triangle& other);
	    bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val);
	private:
		vec3 p0, p1, p2;
};

#endif
