// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410
uniform sampler2D texture;
uniform mat4 model;
uniform mat4 view;
uniform vec3 camera_position;
uniform float material_shininess;
uniform vec3 material_colour;

uniform struct Light {
   vec3 position;
   vec3 intensities; //a.k.a the color of the light
   float ambient;
} light;

in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentTextureCoordinates;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
    /*vec4 diffuse_colour = texture2D(texture, FragmentTextureCoordinates);
    vec3 normal = normalize(mat3(model) * FragmentNormal);
    
    vec3 fragPosition = mat3(model) * FragmentPosition;
    
    vec3 surfaceToLight = normalize(light.position - fragPosition);
	vec3 eyeToSurface = normalize(camera_position - fragPosition);

    float brightness = max(0.0, dot(normal, surfaceToLight));
    vec3 diffuse = diffuse_colour.xyz * brightness;

	vec3 specular = vec3(0.0);
     
    if(brightness > 0.0)
    {
     
    //choose H or R to see the difference
    vec3 R = -normalize(reflect(surfaceToLight, normal));//Reflection
    specular = material_colour * pow(max(0.0, dot(R, eyeToSurface)), material_shininess);
	}

    vec4 surfaceColor = texture2D(texture, FragmentTextureCoordinates);
    FragmentColour = vec4(diffuse + specular, 1.0);*/
	FragmentColour = texture2D(texture, FragmentTextureCoordinates);
}