CPSC453 Assignment 1 - Line and Polygon Geometry
=====================================================
Matthew Sembinelli
10085613
mjsembin@ucalgary.ca

January 28, 2016
=====================================================


Files Included
=====================================================
line_polygon_geometry.cpp
vertex.glsl
vertex_sd.glsl
fragment.glsl
=====================================================


How To Compile And Run
=====================================================
Build with g++:
g++ line_polygon_geometry.cpp -lglfw -lGL -ldl -o Assignment1

Run with:
./Assignment1

*Requires gcc/g++, GLFW and OpenGL 4
=====================================================


How To Use
=====================================================
F1 - Squares and Diamonds Scene
F2 - Parametric Spiral Scene
F3 - Sierpinski Triangle Scene

UP (arrow up) - Increase the number of iterations
DOWN (arrow down) - Decrease the number of iterations

ESC - Exit program
=====================================================


Extra Information
=====================================================
- The number of iterations is capped at 10 but is
  capable of more with no crashes, even on the 
  Sierpinski Triangle scene
- The squares and diamonds scene uses instanced
  rendering, and thus does not require reinitialized
  vertex data. Only one draw call per n objects 
  required. See vertex_sd.glsl to see how this is
  done. This is an interesting technique used
  in rendering hundreds or thousands of similar
  objects at different scales or offsets (think
  blades of grass in a video game)
=====================================================
