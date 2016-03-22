/*
 * Camera.h
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Ray.h"

using namespace glm;

class Camera
{
	public:
	    Camera(GLfloat fov_);
	    void generate_ray(GLuint x, GLuint y, Ray *ray);
	    GLfloat fov;
	private:
	    vec2 normalize_pixel(GLint x, GLint y);
};

#endif



