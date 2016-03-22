/*
 * Ray.h
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

using namespace glm;
class Ray
{
    public:
	    Ray(vec3 origin_, vec3 direction_);
	    vec3 origin;
	    vec3 direction;
};

#endif



