/*
 * Light.cpp
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#include "Light.h"

Light::Light(vec3 point_)
{
	point = point_;
}
void Light::generate_light_ray(const vec3 &scene_intersection, Ray *lray, vec3 *lcolour)
{
    lray->direction = point - scene_intersection;
    lcolour->r = 1.0;
    lcolour->g = 1.0;
    lcolour->b = 1.0;
}



