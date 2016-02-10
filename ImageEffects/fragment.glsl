// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

uniform sampler2DRect texture;
uniform vec3 colour_data;

// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 textureCoords;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
    // write colour output without modification
    if(colour_data[0] == 1.0)
    {
        FragmentColour = texture2DRect(texture, textureCoords);
    }
    else
    {
        vec4 TextureColour = texture2DRect(texture, textureCoords);
         
        float luminance = colour_data[0]*TextureColour[0] + colour_data[1]*TextureColour[1] + colour_data[2]*TextureColour[2];
         
        FragmentColour = vec4(luminance, luminance, luminance, 0);
    }
}
