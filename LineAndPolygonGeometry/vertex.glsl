// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec2 VertexPosition[2];
layout(location = 2) in vec3 VertexColour[2];

// output to be interpolated between vertices and passed to the fragment stage
layout(location = 2) out vec3 Colour[2];

void main()
{
    // assign vertex position without modification
    gl_Position[0] = vec4(VertexPosition[0], 0.0, 1.0);
    gl_Position[1] = vec4(VertexPosition[1], 0.0, 1.0);

    // assign output colour to be interpolated
    Colour[0] = VertexColour[0];
    Colour[1] = VertexColour[1];
}
