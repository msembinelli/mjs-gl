#version 430 core
// Tessellation Evaluation Shader runs once per vertex
// output from the Tesselator. This shader forms the primitives
// to be passed to the fragment shader.

//We want to output lines aka an isoline
layout(isolines) in;
uniform uint bezier_scene;

patch in  vec3 patch_color;
      out vec3 Colour;

#define QUADRATIC_BEZIER 0
#define CUBIC_BEZIER 1

void main( )
{
    Colour = patch_color;
    // receive original positions
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    // set position of new vertex
    float u = gl_TessCoord.x;
    
    
    if (bezier_scene == QUADRATIC_BEZIER)
	    gl_Position = (1.0 - u)*((1.0 - u)*p0 + u*p1) + u*((1.0 - u)*p1 + u*p2);
	else if (bezier_scene == CUBIC_BEZIER)
	    gl_Position = (1.0 - u)*(1.0 - u)*(1.0 - u)*p0 + 3*(1.0 - u)*(1.0 - u)*u*p1 + 3*(1.0 - u)*u*u*p2 + u*u*u*p3;
}
