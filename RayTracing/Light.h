/*
 * Light.h
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "Ray.h"

using namespace glm;

class Light
{
	public:
	    Light(vec3 point_);
	    void generate_light_ray(const vec3 &scene_intersection, Ray *lray, vec3 *lcolour);
	private:
        vec3 point;
};

#endif



