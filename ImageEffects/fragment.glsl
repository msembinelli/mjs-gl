// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

#define M_PI 3.1415926535897932384626433832795

uniform sampler2DRect texture;
uniform mat3 colour_data;

uniform int kernel[9];
vec2 offsets[9] = vec2[](vec2(-1, -1), vec2(0, -1), vec2(1, -1), vec2(-1, 0), vec2(0, 0), vec2(1, 0), vec2(-1, 1), vec2(0, 1), vec2(1, 1));
uniform uint filter_type;

uniform uint gaussian_points;
uniform float sigma;

// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 textureCoords;

// first output is mapped to the uniform framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{ 
    vec4 sum = vec4(0.0);
    switch(filter_type)
    {
        // Gaussian case
        case 4:
            float weight_sum = 0.0;

            // Store in flat arrays to make convolution easier
            float gaussian_kernel[49];
            vec2 gaussian_offset[49];
            
            // Generate nxn gaussian kernel
            float mean = (gaussian_points - 1)/2;
            for (int x = 0; x < gaussian_points; x++) 
            {
                for (int y = 0; y < gaussian_points; y++) {
                    uint index = ((x*gaussian_points) + y);
                    gaussian_kernel[index] = exp( -0.5 * (pow((x - mean)/sigma, 2.0) + pow((y - mean)/sigma,2.0)) ) / (2 * M_PI * sigma * sigma);
                    weight_sum += gaussian_kernel[index];
                    gaussian_offset[index] = vec2(-(int(gaussian_points) - 1)/2 + y, -(int(gaussian_points) - 1)/2 + x);
                }
            }
            
            // normalize kernel
            for (int x = 0; x < gaussian_points; x++) 
            {
                for (int y = 0; y < gaussian_points; y++) {
                     gaussian_kernel[(x*gaussian_points) + y] /= weight_sum;
                }
            }
            
            // perform convolution
            for (int i = 0; i < gaussian_points*gaussian_points; i++) 
            {
                vec4 colour = texture2DRect(texture, textureCoords + gaussian_offset[i]);
                colour = vec4(dot(colour, vec4(vec3(colour_data[0]), 0)), dot(colour, vec4(vec3(colour_data[1]), 0)), dot(colour, vec4(vec3(colour_data[2]), 0)), 0);
                sum += colour * gaussian_kernel[i];
            }
            FragmentColour = sum;
            break;

        // Case for unsharp
        case 3:
            for (int i = 0; i < 9; i++) 
            {
                vec4 colour = texture2DRect(texture, textureCoords + offsets[i]);
                colour = vec4(dot(colour, vec4(vec3(colour_data[0]), 0)), dot(colour, vec4(vec3(colour_data[1]), 0)), dot(colour, vec4(vec3(colour_data[2]), 0)), 0);
                sum += colour * kernel[i];
            }
            FragmentColour = sum;
            break;

        // Cases for vertical and horizontal sobel
        case 2:
        case 1:
            vec4 greyscale = vec4(vec3(0.222, 0.715, 0.072), 0);
            for (int i = 0; i < 9; i++) 
            {
                vec4 colour = texture2DRect(texture, textureCoords + offsets[i]);
                colour = vec4(dot(colour, greyscale), dot(colour, greyscale), dot(colour, greyscale), 0);
                sum += colour * kernel[i];
            }
            FragmentColour = abs(sum);
            break;
        
        // Standard no filter colour effects
        case 0:
            vec4 TextureColour = texture2DRect(texture, textureCoords);

            // Apply colour effect from input matrix
            sum = vec4(dot(TextureColour, vec4(vec3(colour_data[0]), 0)), dot(TextureColour, vec4(vec3(colour_data[1]), 0)), dot(TextureColour, vec4(vec3(colour_data[2]), 0)), 0);
            
            // write colour output
            FragmentColour = sum;
            break;
    } 
}
