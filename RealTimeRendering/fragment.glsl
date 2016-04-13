// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410
uniform sampler2D texture;

// interpolated colour received from vertex stage
//in vec3 Colour;
in vec2 TextureCoords;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
    // write colour output without modification
	vec4 colour = texture2D(texture, TextureCoords);
    FragmentColour = colour;
}
