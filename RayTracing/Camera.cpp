/*
 * Camera.cpp
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#include "Camera.h"
#include "Window.h"

Camera::Camera(GLfloat fov_)
{
	fov = radians(fov_);
}

vec2 Camera::normalize_pixel(GLint x, GLint y)
{
	return vec2((GLint)(x - (WINDOW_WIDTH/2)), (GLint)(y - (WINDOW_HEIGHT/2)));
}

void Camera::generate_ray(GLuint x, GLuint y, Ray *ray)
{
	vec2 normalized_coords = normalize_pixel(x, y);
	// Create ray, origin is always 0 for now
	Ray r(vec3(0.0), normalize(vec3(normalized_coords, -1*((WINDOW_WIDTH/2)/tan(fov/2)))));
	*ray = r;
}



