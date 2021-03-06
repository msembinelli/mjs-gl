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

GLuint Scene::scene_count = 0;

Scene::Scene()
{
	image.Initialize();
	scene_id = scene_count++;
}

void Scene::draw()
{
	Camera camera(50); // 50 degree FOV

	// Loop through each pixel finding the direction vector
	for(GLint y = 0; y < WINDOW_HEIGHT; y++)
	{
		// Trace each ray
		for(GLuint x = 0; x < WINDOW_WIDTH; x++)
		{
			vec3 pixel_colour(0.0);
			Ray ray(vec3(0.0), vec3(0.0));

			camera.generate_ray(x, y, &ray);
			tracer.trace(ray, &pixel_colour, 10, -1);
			// Set imagebuffer pixel colour
			image.SetPixel(x, y, pixel_colour);
		}
	}
}
void Scene::commit()
{
	image.Render();
	string filename = "scene";
	filename.append(to_string(scene_id + 1));
	filename.append(".png");
	image.SaveToFile(filename);
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
    while ((found_pos = str.find(object_names[i], starting_pos)) != string::npos)
    {
      GLint end_pos = str.find('}', found_pos);
      if (object_names[i] == "light")
      {
        float f1, f2, f3, f4, f5, f6;
        sscanf(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f %f %f}", &f1, &f2, &f3, &f4, &f5, &f6);
		Light *l = new Light(vec3(f1, f2, f3), vec3(f4, f5, f6));
	    tracer.lights.push_back(l);
      }
      else if (object_names[i] == "sphere")
      {
        float c1, c2, c3, r, cr1, cr2, cr3, cs1, cs2, cs3, p, ref;
        sscanf(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f %f %f %f %f %f %f %f %f}", &c1, &c2, &c3, &r, &cr1, &cr2, &cr3, &cs1, &cs2, &cs3, &p, &ref);
		Sphere *s = new Sphere(vec3(c1, c2, c3), r, vec3(cr1, cr2, cr3), vec3(cs1, cs2, cs3), p, ref);
	    tracer.objects.push_back(s);
      }
      else if (object_names[i] == "triangle")
      {
        float f1, f2, f3, f4, f5, f6, f7, f8, f9, cr1, cr2, cr3, cs1, cs2, cs3, p, r;
        sscanf(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f}", &f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8, &f9, &cr1, &cr2, &cr3, &cs1, &cs2, &cs3, &p, &r);
		Triangle *t = new Triangle(vec3(f1, f2, f3), vec3(f4, f5, f6), vec3(f7, f8, f9), vec3(cr1, cr2, cr3), vec3(cs1, cs2, cs3), p, r);
	    tracer.objects.push_back(t);
      }
      else if (object_names[i] == "plane")
      {
        float n1, n2, n3, p1, p2, p3, cr1, cr2, cr3, cs1, cs2, cs3, p, r;
        sscanf(str.substr(found_pos, end_pos + 1).c_str(), "%*s { %f %f %f %f %f %f %f %f %f %f %f %f %f %f}", &n1, &n2, &n3, &p1, &p2, &p3,  &cr1, &cr2, &cr3, &cs1, &cs2, &cs3, &p, &r);
		Plane *pl = new Plane(vec3(n1, n2, n3), vec3(p1, p2, p3), vec3(cr1, cr2, cr3), vec3(cs1, cs2, cs3), p, r);
	    tracer.objects.push_back(pl);
      }
      starting_pos = end_pos+1;
    }
  }

}




