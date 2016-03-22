#include "rt_modules.h"

using namespace std;

/***Scene***/

void Scene::draw()
{
	Tracer tracer;
	Camera camera(60); // 60 degree FOV
	tracer.lights.push_back(Light(glm::vec3(0, 2.5, -7.75)));
	tracer.objects.push_back(Sphere(glm::vec3(0.9, -1.925, -6.69), 0.825));
	
	// Loop through each pixel finding the direction vector
	for(GLint y = 0; y < WINDOW_HEIGHT; y++)
	{
		// Trace each ray
		for(GLuint x = 0; x < WINDOW_WIDTH; x++)
		{
			glm::vec3 *pixel_colour;
			Ray *ray;
			
			camera.generate_ray(x, y, ray);
			tracer.trace(ray, pixel_colour, 1);
			// Set imagebuffer pixel colour
			image.SetPixel(x, y, *pixel_colour);
			
			// Cleanup
			delete ray;
			delete pixel_colour;
		}
	}
}
void Scene::commit()
{
	image.Render();
}

/***Tracer***/

void Tracer::trace(Ray &ray, glm::vec3 *pixel_colour, GLuint recursion_depth)
{
	if(recursion_depth == 0)
	{
		pixel_colour = new glm::vec3(0.0);
		return;
	}
	
	GLfloat min_t_val, t_val;
	GLint intersect_obj_index = -1;
	glm::vec3 intersection_point, tmp_point;
	t_val = 1E6;
	for(GLuint i= 0; i < objects.size(); i++)
	{
		// Find intersection points of each object in the scene with viewing ray
		if(objects.at(i)->intersect(ray, &t_val, &tmp_point))
		{
			if(t_val < min_t_val && t_val > 0)
			{
				min_t_val = t_val;
				intersect_obj_index = i;
				intersection_point = tmp_point;
			}
		}
	}
	
	if(intersect_obj_index < 0) //No intersec, return black
	{
		pixel_colour = new glm::vec3(0.0);
		return;
	}
	
	Ray *lray;
	bool in_shadow = false;
	for(GLuint i= 0; i < lights.size(); i++)
	{
		lights.at(i)->generate_light_ray(intersection_point, lray, lcolour);
	    for(GLuint i= 0; i < objects.size(); i++)
	    {
			if(i == intersect_obj_index)
			{
				continue; // Exclude the object we are shading
			}
	    	// Find intersection points of each object in the scene with light ray
	    	if(objects.at(i)->intersect(lray))
	    	{
				in_shadow = true;
				break;
	    	}
	    }
	}
	if(!in_shadow)
	    *pixel_colour += shading(intersection_point, lray, lcolour);
	
	//Handle mirror reflection
	
}

/***Camera***/

Camera::Camera(GLfloat fov_)
{
	fov = glm::radians(fov_);
}

glm::vec2 Camera::normalize_pixel(GLuint x, GLuint y)
{
	return glm::vec2((x - (WINDOW_WIDTH/2)), (y - (WINDOW_HEIGHT/2)));
}

void Camera::generate_ray(GLuint x, GLuint y, Ray *ray)
{
	glm::vec2 normalized_coords = normalize_pixel(x, y);
	// Create ray, origin is always 0 for now
	ray = new Ray(glm::vec3(0.0), glm::normalize(glm::vec3(normalized_coords, -1*((WINDOW_WIDTH/2)/glm::tan(fov/2)))));
}

/***Ray***/

Ray::Ray(glm::vec3 origin_, glm::vec3 direction_)
{
	origin = origin_;
	direction = direction_;
}

/***Light***/

Light::Light(glm::vec3 point_)
{
	point = point_;
}
void Light::generate_light_ray(glm::vec3 &scene_intersection, Ray *lray, glm::vec3 *lcolour)
{
	
}