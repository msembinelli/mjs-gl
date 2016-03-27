#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Ray.h"
using namespace glm;

class Object
{
    public:
	    Object(vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_, GLfloat reflectance_);
	    vec3 diffuse_colour;
		vec3 specular_colour;
		GLfloat reflectance;
		GLfloat phong_exponent;
	    virtual bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val) = 0;
	    virtual vec3 normal(const vec3 &intersection_point) = 0;
};

class Sphere : public Object
{
	public:
	    Sphere(vec3 center_, GLfloat radius_, vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_, GLfloat reflectance_);
	    bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val);
	    vec3 normal(const vec3 &intersection_point);
	private:
		vec3 center;
	    GLfloat radius;
};

class Plane : public Object
{
	public:
	    Plane(vec3 normal_, vec3 point_, vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_, GLfloat reflectance_);
	    bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val);
	    vec3 normal(const vec3 &intersection_point);
	private:
		vec3 p_normal;
	    vec3 point;
};

class Triangle : public Object
{
	public:
	    Triangle(vec3 p0_, vec3 p1_, vec3 p2_, vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_, GLfloat reflectance_);
	    bool intersect(const Ray &ray, vec3 *point, GLfloat *t_val);
	    vec3 normal(const vec3 &intersection_point);
	private:
		vec3 p0, p1, p2;
};

#endif
