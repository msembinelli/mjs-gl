#version 430 core
// Tessellation Control Shader runs once per input vertex.
// It tells the Tessellator how many vertices to create
layout (vertices = 4) out;
// output patches with a particular vertex count.

in  vec3 Colour [];
patch out vec3 patch_color;

void main( )
{
  //pass the incoming vertex position down the pipeline
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  // Since we are dealing with isolines, the following tessellation parameters
  // need to be set. If you are tessellating triangles, etc these parameters will
  // have different meanings.
  gl_TessLevelOuter[0] = 1;
  gl_TessLevelOuter[1] = 50;
  patch_color = Colour[gl_InvocationID];
}
