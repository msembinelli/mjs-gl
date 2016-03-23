/*
 * Tracer.cpp
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#include "Tracer.h"

#include <iostream>
void Tracer::trace(const Ray &ray, glm::vec3 *pixel_colour, GLuint recursion_depth)
{
	if(recursion_depth == 0)
	{
		return;
	}

	GLfloat min_t_val, t_val;
	GLint intersect_obj_index = -1;
	glm::vec3 intersection_point, tmp_point;
	min_t_val = 1E6;
	for(GLuint i= 0; i < objects.size(); i++)
	{
		// Find intersection points of each object in the scene with viewing ray
		if(objects.at(i)->intersect(ray, &tmp_point, &t_val))
		{
			if(t_val < min_t_val && t_val > 0)
			{
				min_t_val = t_val;
				intersect_obj_index = i;
				intersection_point = tmp_point;
			}
		}
	}

	if(intersect_obj_index < 0)
	{
		return;
	}

	Ray lray(glm::vec3(0.0), glm::vec3(0.0));
	vec3 lcolour(0.0);
	bool in_shadow = false;
	for(GLuint i= 0; i < lights.size(); i++)
	{
		lights.at(i)->generate_light_ray(intersection_point, &lray, &lcolour);
	    for(GLuint i= 0; i < objects.size(); i++)
	    {
			if(i == intersect_obj_index)
			{
				continue; // Exclude the object we are shading
			}
	    	// Find intersection points of each object in the scene with light ray
	    	if(objects.at(i)->intersect(lray, &tmp_point, &t_val))
	    	{
				in_shadow = true;
				break;
	    	}
	    }
	}
	if(!in_shadow)
	{
	    *pixel_colour = shade(intersection_point, objects.at(intersect_obj_index), lray, lcolour);
	}

	//Handle mirror reflection

}

vec3 Tracer::shade(const vec3 &intersection, const Object *object, const Ray &ray, const vec3 &light_colour)
{

	return object->diffuse_colour;
}



