// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexTextureCoordinates;

out vec3 FragmentPosition;
out vec3 FragmentNormal;
out vec2 FragmentTextureCoordinates;

//uniforms
// add transformation uniforms
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    // assign vertex position without modification
    gl_Position = proj*view*model*vec4(VertexPosition, 1.0);

	FragmentTextureCoordinates = VertexTextureCoordinates;
	FragmentNormal = VertexNormal;
	FragmentPosition = VertexPosition;
}
