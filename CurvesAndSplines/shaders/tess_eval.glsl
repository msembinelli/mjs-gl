#version 430 core
// Tessellation Evaluation Shader runs once per vertex
// output from the Tesselator. This shader forms the primitives
// to be passed to the fragment shader.

//We want to output lines aka an isoline
layout(isolines) in;

void main( )
{
    // receive original positions
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    // set position of new vertex
    float u = gl_TessCoord.x;
    if(u < 0.334)
    {
        gl_Position = p0;
    }
    else if(u > 0.667)
    {
        gl_Position = p2;
    }
    else
    {
        gl_Position = p1;
    }
}
