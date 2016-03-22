/*
 * Light.cpp
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#include "Light.h"

Light::Light(vec3 point_, vec3 intensity_)
{
	point = point_;
	intensity = intensity_;
}
void Light::generate_light_ray(const vec3 &scene_intersection, Ray *lray, vec3 *lcolour)
{
    lray->direction = point - scene_intersection;
	lray->origin = point;
    *lcolour = intensity;
}



