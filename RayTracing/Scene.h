/*
 * Scene.h
 *
 *  Created on: Mar 21, 2016
 *      Author: matt
 */
#ifndef SCENE_H
#define SCENE_H

#include "ImageBuffer.h"
#include "Tracer.h"
#include <string>
//TODO, make window height and width a property of the scene, base the normalization off of this
using namespace std;

class Scene
{
	public:
	    ImageBuffer image;
	    Tracer tracer;
	    Scene();
	    void parse(string file);
	    void draw();
	    void commit();
};

#endif



