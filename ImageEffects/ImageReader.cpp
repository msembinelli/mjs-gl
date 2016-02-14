// ==========================================================================
// Image File Reading Support Functions
//  - requires the Magick++ development libraries: http://www.imagemagick.org
//
// Note: This file will not compile by itself! The snippets below are meant
// to be added to the template application distributed for Assignment #1.
// You may use this code (or not) however you see fit for your work.
//
// Author:  Sonny Chan, University of Calgary
// Date:    January 2016
// ==========================================================================

#include <Magick++.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

// --------------------------------------------------------------------------
// Functions to set up OpenGL objects for storing image data

struct MyTexture
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  textureName;
    
    // dimensions of the image stored in this texture
    GLuint  width, height;
    
    // initialize object names to zero (OpenGL reserved value)
    MyTexture() : textureName(0), width(0), height(0)
    {}
};

// use the Magick++ library to load an image with a given file name into
// an OpenGL rectangle texture
bool InitializeTexture(MyTexture *texture, const string &imageFileName)
{
    Magick::Image myImage;
    
    // try to read the provided image file
    try {
        myImage.read(imageFileName);
    }
    catch (Magick::Error &error) {
        cout << "Magick++ failed to read image " << imageFileName << endl;
        cout << "ERROR: " << error.what() << endl;
        return false;
    }
    
    // store the image width and height into the texture structure
    texture->width = myImage.columns();
    texture->height = myImage.rows();
    
    // create a Magick++ pixel cache from the image for direct access to data
    Magick::Pixels pixelCache(myImage);
    Magick::PixelPacket *pixels;
    pixels = pixelCache.get(0, 0, texture->width, texture->height);
    
    // determine the number of stored bytes per pixel channel in the cache
    GLenum channelDataType;
    switch (sizeof(Magick::Quantum)) {
        case 4:     channelDataType = GL_UNSIGNED_INT;      break;
        case 2:     channelDataType = GL_UNSIGNED_SHORT;    break;
        default:    channelDataType = GL_UNSIGNED_BYTE;
    }
    
    // create a texture name to associate our image data with
    if (!texture->textureName)
        glGenTextures(1, &texture->textureName);
    
    // bind the texture as a "rectangle" to access using image pixel coordinates
    glBindTexture(GL_TEXTURE_RECTANGLE, texture->textureName);
    
    // send image pixel data to OpenGL texture memory
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, texture->width, texture->height,
                 0, GL_BGRA, channelDataType, pixels);
    
    // unbind this texture
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    
    return !CheckGLErrors();
}

