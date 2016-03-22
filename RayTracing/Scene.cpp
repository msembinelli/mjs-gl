/*
 * Scene.cpp
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#include "Scene.h"

#include "Tracer.h"
#include "Camera.h"
#include "Primitives.h"
#include "Ray.h"
#include "Window.h"

#include <fstream>
#include <vector>

using namespace glm;
using namespace std;

Scene::Scene()
{
	image.Initialize();
}

void Scene::draw()
{
	Camera camera(60); // 60 degree FOV
	Light l(vec3(0, 2.5, -7.75));
	tracer.lights.push_back(&l);

	Sphere s(vec3(0.9, -1.925, -6.69), 0.825);
	tracer.objects.push_back(&s);
	Triangle t1(vec3(-0.4, -2.75, -9.55), vec3(-0.93, 0.55, -8.51), vec3(0.11, -2.75, -7.98));
	tracer.objects.push_back(&t1);
	Triangle t2(vec3(0.11, -2.75, -7.98), vec3(-0.93, 0.55, -8.51), vec3(-1.46, -2.75, -7.47));
	tracer.objects.push_back(&t2);

	// Loop through each pixel finding the direction vector
	for(GLint y = 0; y < WINDOW_HEIGHT; y++)
	{
		// Trace each ray
		for(GLuint x = 0; x < WINDOW_WIDTH; x++)
		{
			vec3 pixel_colour(0.0);
			Ray ray(vec3(0.0), vec3(0.0));

			camera.generate_ray(x, y, &ray);
			tracer.trace(ray, &pixel_colour, 1);
			// Set imagebuffer pixel colour
			image.SetPixel(x, y, pixel_colour);
		}
	}
}
void Scene::commit()
{
	image.Render();
}

/*enum PrimitiveState
{
	LIGHT,
	SPHERE,
	TRIANGLE,
	PLANE,
	PRIMITIVE_MAX
};

enum BracketState
{
	LIGHT,
	SPHERE,
	TRIANGLE,
	PLANE,
	PRIMITIVE_MAX
};

void Scene::parse(string file)
{
	ifstream input(file.c_str());
	PrimitiveState parse_state = PRIMITIVE_MAX;
    for(string line; getline(input, line);)
    {
    	if(line.compare("\n") == 0 || line[0] == '#')
    	{
    		continue;
    	}
        if(line.find("light") != std::string::npos)
        {
        	parse_state = LIGHT;
        }
        else if(line.find("sphere") != std::string::npos)
        {
        	parse_state = SPHERE;
        }
        else if(line.find("triangle") != std::string::npos)
        {
        	parse_state = TRIANGLE;
        }
        else if(line.find("plane") != std::string::npos)
        {
        	parse_state = PLANE;
        }
    }
}*/




