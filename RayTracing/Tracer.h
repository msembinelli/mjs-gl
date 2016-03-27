/*
 * Tracer.h
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#ifndef TRACER_H
#define TRACER_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>

#include "Light.h"
#include "Ray.h"
#include "Primitives.h"

using namespace std;
using namespace glm;

class Tracer
{
    public:
	    vector<Light*> lights;
	    vector<Object*> objects;
	    void trace(const Ray &ray, vec3 *colour, GLuint recursion_depth, GLint recursive_object_index);
	    vec3 shade(const vec3 &intersection, const GLint &object_index, const Ray &cray, const Ray &ray, const vec3 &light_colour, const bool &in_shadow);
};


#endif


