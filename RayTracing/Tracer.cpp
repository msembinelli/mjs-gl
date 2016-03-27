/*
 * Tracer.cpp
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#include "Tracer.h"

void Tracer::trace(const Ray &ray, glm::vec3 *pixel_colour, GLuint recursion_depth, GLint recursive_object_index)
{
	if(recursion_depth == 0)
	{
		return;
	}

	GLfloat min_t_val, t_val;
	GLint intersect_obj_index = -1;
	vec3 intersection_point, tmp_point;
	min_t_val = 1E6;
	for(GLuint i= 0; i < objects.size(); i++)
	{
		// For reflected rays, exclude object reflected ray was generated from
		if( i != recursive_object_index)
		{
		    // Find intersection points of each object in the scene with viewing ray
		    if(objects.at(i)->intersect(ray, &tmp_point, &t_val))
		    {
			    if(t_val < min_t_val && t_val > std::numeric_limits<float>::epsilon())
			    {
				    min_t_val = t_val;
				    intersect_obj_index = i;
				    intersection_point = tmp_point;
			    }
		    }
		}
	}

	if(intersect_obj_index < 0)
	{
		return;
	}

	Ray lray(vec3(0.0), vec3(0.0));
	vec3 lcolour(0.0);
	GLint shadow_obj_index = -1;
	vec3 shadow_point;
	min_t_val = 1E6;
	bool in_shadow = false;
	for(GLint i= 0; i < lights.size(); i++)
	{
		lights.at(i)->generate_light_ray(intersection_point, &lray, &lcolour);
	    for(GLint j= 0; j < objects.size(); j++)
	    {
	    	// Exclude the object we are shading
			if(j != intersect_obj_index)
			{
	    	    // Find intersection points of each object in the scene with light ray
	    	    if(objects.at(j)->intersect(lray, &tmp_point, &t_val))
	    	    {
				    if(t_val < min_t_val && t_val > 0)
				    {
					    min_t_val = t_val;
					    shadow_obj_index = j;
					    shadow_point = tmp_point;
				    }
	    	    }
	    	}
	    }
	}

	GLfloat t_light =  length(lray.direction);
	GLfloat t_tmp =  length(shadow_point - intersection_point);
	if(shadow_obj_index >= 0 && t_tmp < t_light && min_t_val > std::numeric_limits<float>::epsilon())
	{
		in_shadow = true;
	}

	*pixel_colour += shade(intersection_point, intersect_obj_index, ray, lray, lcolour, in_shadow);

	//Handle mirror reflection
	if(objects.at(intersect_obj_index)->reflectance > 0)
	{
		Ray reflection_ray(intersection_point, reflect(ray.direction, objects.at(intersect_obj_index)->normal(intersection_point)));
		vec3 reflection_colour(0.0);
		trace(reflection_ray, &reflection_colour, recursion_depth - 1, intersect_obj_index);
		*pixel_colour += (objects.at(intersect_obj_index)->reflectance * reflection_colour);
	}

    if(pixel_colour->x > 1.0) pixel_colour->x = 1.0;
    if(pixel_colour->y > 1.0) pixel_colour->y = 1.0;
    if(pixel_colour->z > 1.0) pixel_colour->z = 1.0;

}

vec3 Tracer::shade(const vec3 &intersection, const GLint &object_index, const Ray &cray, const Ray &lray, const vec3 &light_colour, const bool &in_shadow)
{
	vec3 colour(0.0);
	Object *object = objects.at(object_index);

	// Ambient
    colour += object->diffuse_colour * (GLfloat)0.4;

    if(in_shadow)
    	return colour;

    // Diffuse
    colour += object->diffuse_colour * (light_colour*(GLfloat)glm::max((GLfloat)0.0, dot(normalize(object->normal(intersection)), normalize(lray.direction))));

    // Specular
    if(object->phong_exponent > 0)
    {
    	if (dot(lray.direction, object->normal(intersection)) > 0.0)
    	{
            vec3 reflected = normalize(reflect(lray.direction , object->normal(intersection)));
            colour += object->specular_colour * (light_colour*pow((GLfloat)glm::max((GLfloat)0.0, dot(normalize(cray.direction), reflected)), object->phong_exponent));
    	}
    }

    for(GLuint i = 0; i < colour.length(); i++) { if(colour[i] > 1.0) colour[i] = 1.0; }

	return colour;
}



