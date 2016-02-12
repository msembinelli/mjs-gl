// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

uniform sampler2DRect texture;
uniform mat3 colour_data;

// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 textureCoords;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
    vec4 TextureColour = texture2DRect(texture, textureCoords);
	
    // Apply colour effect from input matrix
    vec3 TmpColours = vec3(dot(TextureColour, vec4(vec3(colour_data[0]), 0)), dot(TextureColour, vec4(vec3(colour_data[1]), 0)), dot(TextureColour, vec4(vec3(colour_data[2]), 0)));

    // write colour output without modification
    FragmentColour = vec4(TmpColours, 0);
}
