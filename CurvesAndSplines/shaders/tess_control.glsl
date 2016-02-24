#version 430 core
// Tessellation Control Shader runs once per input vertex.
// It tells the Tessellator how many vertices to create

// output patches with a particular vertex count.

void main( )
{
  //pass the incoming vertex position down the pipeline

  // Since we are dealing with isolines, the following tessellation parameters
  // need to be set. If you are tessellating triangles, etc these parameters will
  // have different meanings.
}
