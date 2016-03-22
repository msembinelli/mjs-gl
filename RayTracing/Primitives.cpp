
#include "Primitives.h"
using namespace std;

Sphere::Sphere(vec3 center_, GLfloat radius_)
{
    center = center_;
    radius = radius_;
}

bool Sphere::intersect(const Ray &ray, vec3 *point, GLfloat *t_val)
{
	GLfloat a = dot(ray.direction, ray.direction);
	GLfloat b = 2*dot(ray.direction, -center);
	GLfloat c = dot(-center, -center) - (radius*radius);

	// if discriminant is positive, intersection exists
	GLfloat discriminant = (b*b) - (4*a*c);
	if(discriminant < 0)
	{
		return false;
	}
	GLfloat t1 = (((-1*b) + sqrt(discriminant))/2*a);
	GLfloat t2 = (((-1*b) - sqrt(discriminant))/2*a);
	if(t1 < t2)
	{
		*t_val = t1;
	}
	else
	{
		*t_val = t2;
	}
	*point = (*t_val * ray.direction);

    return true;
}

Triangle::Triangle(vec3 p0_, vec3 p1_, vec3 p2_)
{
	p0 = p0_;
	p1 = p1_;
	p2 = p2_;
}

bool Triangle::intersect(const Ray &ray, vec3 *point, GLfloat *t_val)
{
    vec3 normal = normalize(cross((p1 - p0), (p2 - p0)));

    GLfloat w = dot(-p0, normal);
    GLfloat a = dot(ray.direction, normal);
	if(a == 0) //TODO handle line contained within plane?
		return false;
    *t_val = w/a;
    *point = (*t_val * ray.direction);

    GLfloat xb_xa = p1.x - p0.x;
    GLfloat yb_ya = p1.y - p0.y;
    GLfloat xc_xa = p2.x - p0.x;
    GLfloat yc_ya = p2.y - p0.y;
    GLfloat xp_xa = point->x - p0.x;
    GLfloat yp_ya = point->y - p0.y;

    GLfloat denominator = determinant(mat2(xb_xa, xc_xa, yb_ya, yc_ya));
    GLfloat a1 = determinant(mat2(xp_xa, xc_xa, yp_ya, yc_ya));
    GLfloat a2 = determinant(mat2(xb_xa, xp_xa, yb_ya, yp_ya));

    GLfloat beta = a1/denominator;
    GLfloat sigma = a2/denominator;

    GLfloat alpha = 1 - beta - sigma;

    if(alpha < 0)
    	return false;

    return true;
}

Plane::Plane(vec3 normal_, vec3 point_)
{
	normal = normal_;
	point = point_;
}

bool Plane::intersect(const Ray &ray, vec3 *point, GLfloat *t_val)
{
    GLfloat w = dot(-point, normal);
    GLfloat a = dot(ray.direction, normal);
	if(a == 0) //TODO handle line contained within plane?
		return false;
    *t_val = w/a;
    *point = (*t_val * ray.direction);

    return true;
}
