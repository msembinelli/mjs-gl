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
layout(location = 2) in vec2 ControlPosition;
uniform bool control_points;
uniform bool control_lines;
uniform float scroll_pos;

// output to be interpolated between vertices and passed to the fragment stage
out vec3 Colour;

void main()
{
    if(control_lines)
        gl_Position = vec4(ControlPosition, 0.0, 1.0);
    else
    {
        vec2 tmp = vec2((VertexPosition[0] - scroll_pos), VertexPosition[1]);
        gl_Position = vec4(tmp, 0.0, 1.0);
    }

    // assign output colour to be interpolated
    if(!control_points && !control_lines)
    {
        Colour = vec3(1.0, 1.0, 1.0);
    }
    else
    {
        if(control_points)
        {
            gl_PointSize = 4.0;
            Colour = VertexColour;
        }
        else if(control_lines)
            Colour = vec3(0.3, 0.8, 0.6);
    }
}
