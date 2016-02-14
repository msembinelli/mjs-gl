CPSC453 Assignment 2 - Image Effects
=====================================================
Matthew Sembinelli
10085613
mjsembin@ucalgary.ca

February 3, 2016
=====================================================


Files Included
=====================================================
image1-mandrill.png
image2-uclogo.png
image3-aerial.jpg
image4-thirsk.jpg
image5-pattern.png
image6-capstone.jpeg
image_effects.cpp
fragment.glsl
vertex.glsl
=====================================================


How To Compile And Run
=====================================================
Build with g++:
g++ image_effects.cpp -lglfw -lGraphicsMagick++ -lGL -ldl -I/usr/include/GraphicsMagick -o Assignment2

Run with:
./Assignment2

*Requires gcc/g++, GLFW, GraphicsMagick++ and OpenGL 4
=====================================================


How To Use
=====================================================
Keyboard Input:
1-6 - Display images 1-6
F1 - Normal Colour
F2 - Greyscale 1
F3 - Greyscale 2
F4 - Greyscale 3
F5 - Sepia
F - Rotate through filters (vertical, horizontal sobel, unsharp)
G - Rotate through gaussian (3x3, 5x5, 7x7)
LEFT - Rotate image left
RIGHT - Rotate image right
UP/SCROLL UP - Zoom in
DOWN/SCROLL DOWN - Zoom out
CLICK/HOLD/DRAG - Pan/move image
=====================================================
