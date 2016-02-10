// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec3 VertexColour;
layout(location = 2) in vec2 VertexTextureCoords;
uniform vec4 transformation_data;

// output to be interpolated between vertices and passed to the fragment stage
out vec3 Colour;
out vec2 textureCoords;

void main()
{
    mat2 rotation = mat2(cos(transformation_data[3]), sin(transformation_data[3]), -sin(transformation_data[3]), cos(transformation_data[3]));
    vec2 TmpVertexPosition = rotation * VertexPosition;
    TmpVertexPosition *= transformation_data[2];
    TmpVertexPosition[0] += transformation_data[0];
    TmpVertexPosition[1] += transformation_data[1];
    
    // assign vertex position without modification
    gl_Position = vec4(TmpVertexPosition, 0.0, 1.0);

    // assign output colour to be interpolated
    Colour = VertexColour;
    textureCoords = VertexTextureCoords;
}
