
#include "Primitives.h"
using namespace std;

Object::Object(vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_, GLfloat reflectance_)
{
    diffuse_colour = diffuse_colour_;
    specular_colour = specular_colour_;
    phong_exponent = phong_exponent_;
    reflectance = reflectance_;
}

Sphere::Sphere(vec3 center_, GLfloat radius_, vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_, GLfloat reflectance_)
: Object(diffuse_colour_, specular_colour_, phong_exponent_, reflectance_)
{
    center = center_;
    radius = radius_;
}

bool Sphere::intersect(const Ray &ray, vec3 *point, GLfloat *t_val)
{
	GLfloat a = dot(ray.direction, ray.direction);
	GLfloat b = 2*dot(ray.direction, -center) + 2*dot(ray.origin, ray.direction);
	GLfloat c = 2*dot(ray.origin, -center) + dot(-center, -center) + dot(ray.origin, ray.origin) - (radius*radius);

	// if discriminant is positive, intersection exists
	GLfloat discriminant = (b*b) - (4*a*c);
	if(discriminant < 0)
	{
		return false;
	}
	GLfloat t1 = (((-1*b) + sqrt(discriminant))/(2*a));
	GLfloat t2 = (((-1*b) - sqrt(discriminant))/(2*a));
	if(t1 < t2)
	{
		*t_val = t1;
	}
	else
	{
		*t_val = t2;
	}
	*point = (*t_val * ray.direction) + ray.origin;

    return true;
}

vec3 Sphere::normal(const vec3 &intersection_point)
{
	return normalize(intersection_point - center);
}

Triangle::Triangle(vec3 p0_, vec3 p1_, vec3 p2_, vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_,  GLfloat reflectance_)
: Object(diffuse_colour_, specular_colour_, phong_exponent_, reflectance_)
{
	p0 = p0_;
	p1 = p1_;
	p2 = p2_;
}

bool Triangle::intersect(const Ray &ray, vec3 *point, GLfloat *t_val)
{
	vec3 p0_p1 = (p1 - p0);
	vec3 p0_p2 = (p2 - p0);
	vec3 pvec = cross(ray.direction, p0_p2);
	GLfloat det = dot(p0_p1, pvec);

	if(fabs(det) < numeric_limits<float>::epsilon())
		return false;

	vec3 tvec = ray.origin - p0;

	GLfloat gamma = dot(tvec, pvec) / det;
	if(gamma < 0 || gamma > 1)
		return false;

	vec3 qvec = cross(tvec, p0_p1);
	GLfloat beta = dot(ray.direction, qvec) / det;
	if(beta < 0 || beta > (1 - gamma))
		return false;

	*t_val = dot(p0_p2, qvec) / det;
	*point = (*t_val * ray.direction) + ray.origin;

    return true;
}

vec3 Triangle::normal(const vec3 &intersection_point)
{
	return normalize(cross((p0 - p1), (p1 - p2)));
}

Plane::Plane(vec3 normal_, vec3 point_, vec3 diffuse_colour_, vec3 specular_colour_, GLfloat phong_exponent_,  GLfloat reflectance_)
: Object(diffuse_colour_, specular_colour_, phong_exponent_, reflectance_)
{
	p_normal = normal_;
	point = point_;
}

bool Plane::intersect(const Ray &ray, vec3 *point, GLfloat *t_val)
{
    GLfloat w = dot(this->point - ray.origin, p_normal);
    GLfloat a = dot(ray.direction, p_normal);
	if(fabs(a) < numeric_limits<float>::epsilon())
		return false;
    *t_val = w/a;
    if(*t_val < 0)
       return false;
    *point = (*t_val * ray.direction) + ray.origin;

    return true;
}

vec3 Plane::normal(const vec3 &intersection_point)
{
	return normalize(p_normal);
}
