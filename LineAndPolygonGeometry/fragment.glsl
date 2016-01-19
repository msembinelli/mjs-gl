// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage
in vec3 Colour[2];

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour[2];

void main(void)
{
    // write colour output without modification
    FragmentColour[0] = vec4(Colour[0], 0);
    FragmentColour[1] = vec4(Colour[1], 0);
}
