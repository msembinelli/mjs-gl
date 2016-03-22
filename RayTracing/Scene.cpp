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
#include <streambuf>

using namespace glm;
using namespace std;

Scene::Scene()
{
	image.Initialize();
}

void Scene::draw()
{
	Camera camera(60); // 60 degree FOV

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

void Scene::parse(string file)
{
  ifstream input(file.c_str());
  string str, line;
  while (getline(input, line))
  {
    if (line.compare("\n") == 0 || line[0] == '#')
    {
      continue;
    }
    str += line;
  }
  string object_names[4] = { "light", "sphere", "triangle", "plane" };
  for (GLint i = 0; i < 4; i++)
  {
    GLint starting_pos = 0;
    GLint found_pos = 0;
    while ((found_pos = str.find(object_names[i], starting_pos)) != std::string::npos)
    {
      GLint end_pos = str.find('}', found_pos);
      if (object_names[i] == "light")
      {
        float f1, f2, f3;
        sscanf_s(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f}", &f1, &f2, &f3);
		Light l(vec3(f1, f2, f3));
	    tracer.lights.push_back(&l);
      }
      else if (object_names[i] == "sphere")
      {
        float c1, c2, c3, r;
        sscanf_s(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f}", &c1, &c2, &c3, &r);
		Sphere s(vec3(c1, c2, c3), r);
	    tracer.objects.push_back(&s);
      }
      else if (object_names[i] == "triangle")
      {
        float f1, f2, f3, f4, f5, f6, f7, f8, f9;
        sscanf_s(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f %f %f %f %f %f}", &f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8, &f9);
		Triangle t(vec3(f1, f2, f3), vec3(f4, f5, f6), vec3(f7, f8, f9));
	    tracer.objects.push_back(&t);
      }
      else if (object_names[i] == "plane")
      {
        float n1, n2, n3, p1, p2, p3;
        sscanf_s(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f %f %f}", &n1, &n2, &n3, &p1, &p2, &p3);
		Plane p(vec3(n1, n2, n3) vec3(p1, p2, p3));
	    tracer.objects.push_back(&p);
      }
      starting_pos = end_pos+1;
    }
  }
}




