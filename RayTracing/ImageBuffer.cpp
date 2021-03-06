// ==========================================================================
// Pixel Rendering and Image Saving Support Code
//  - requires the OpenGL Mathmematics (GLM) library: http://glm.g-truc.net
//  - requires the Magick++ development libraries: http://www.imagemagick.org
//  -   or the FreeImage library: http://freeimage.sourceforge.net
//
// You may use this code (or not) however you see fit for your work.
//
// Set the #defines below to choose the image library you have installed on
// your system, that you will be using for your assignment. Then compile and
// link this source file with your project.
//
// Authors: Sonny Chan, Alex Brown
//          University of Calgary
// Date:    February-March 2016
// ==========================================================================

#include "ImageBuffer.h"

#include <iostream>
#include <glm/common.hpp>

// --------------------------------------------------------------------------
// Set these defines to choose which image library to use for saving image
// files to disk. Obviously, you shouldn't set both!

#define USE_IMAGEMAGICK
// #define USE_FREEIMAGE

#ifdef USE_IMAGEMAGICK
#include <Magick++.h>
#endif
#ifdef USE_FREEIMAGE
#include <FreeImage.h>
#endif

using namespace std;
using namespace glm;

// --------------------------------------------------------------------------

ImageBuffer::ImageBuffer()
    : m_textureName(0), m_framebufferObject(0),
      m_width(0), m_height(0), m_modified(false)
{
}

ImageBuffer::~ImageBuffer()
{
    if (m_framebufferObject)    glDeleteFramebuffers(1, &m_framebufferObject);
    if (m_textureName)          glDeleteTextures(1, &m_textureName);
}

void ImageBuffer::ResetModified()
{
    m_modified = false;
    m_modifiedLower = m_height;
    m_modifiedUpper = 0;
}

// --------------------------------------------------------------------------

bool ImageBuffer::Initialize()
{
    // retrieve the current viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_width = viewport[2];
    m_height = viewport[3];

    // allocate image data
    m_imageData.resize(m_width * m_height);
    for (int i = 0, k = 0; i < m_height; ++i)
        for (int j = 0; j < m_width; ++j, ++k)
        {
            int p = (i >> 4) + (j >> 4);
            float c = 0.2 + ((p & 1) ? 0.1 : 0.0);
            m_imageData[k] = vec3(c);
        }

    // allocate texture object
    if (!m_textureName)
        glGenTextures(1, &m_textureName);
    glBindTexture(GL_TEXTURE_RECTANGLE, m_textureName);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, m_width, m_height, 0, GL_RGB,
                 GL_FLOAT, &m_imageData[0]);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    ResetModified();

    // allocate framebuffer object
    if (!m_framebufferObject)
        glGenFramebuffers(1, &m_framebufferObject);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferObject);
    glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureName, 0);

    // check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        cout << "ImageBuffer ERROR: Framebuffer object not complete!" << endl;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return status == GL_FRAMEBUFFER_COMPLETE;
}

// --------------------------------------------------------------------------

void ImageBuffer::SetPixel(int x, int y, vec3 colour)
{
    int index = y * m_width + x;
    m_imageData[index] = colour;

    // mark that something was changed
    m_modified = true;
    m_modifiedLower = std::min(m_modifiedLower, y);
    m_modifiedUpper = std::max(m_modifiedUpper, y+1);
}

// --------------------------------------------------------------------------

void ImageBuffer::Render()
{
    if (!m_framebufferObject) return;

    // check for modifications to the image data and update texture as needed
    if (m_modified)
    {
        int sizeY = m_modifiedUpper - m_modifiedLower;
        int index = m_modifiedLower * m_width;

        // bind texture and copy only the rows that have been changed
        glBindTexture(GL_TEXTURE_RECTANGLE, m_textureName);
        glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, m_modifiedLower, m_width,
                        sizeY, GL_RGB, GL_FLOAT, &m_imageData[index]);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);

        // mark that we've updated the texture
        ResetModified();
    }

    // bind the framebuffer object with our texture in it and copy to screen
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferObject);
    glBlitFramebuffer(0, 0, m_width, m_height,
                      0, 0, m_width, m_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// --------------------------------------------------------------------------

bool ImageBuffer::SaveToFile(const string &imageFileName)
{
    if (m_width == 0 || m_height == 0)
    {
        cout << "ImageBuffer ERROR: Trying to save uninitialized image!" << endl;
        return false;
    }
    cout << "ImageBuffer saving image to " << imageFileName << "..." << endl;

#ifdef USE_IMAGEMAGICK
    using namespace Magick;

    // allocate an image object the same size as our buffer
    Image myImage(Geometry(m_width, m_height), "black");

    // copy the image data from our memory buffer into the Magick++ one.
    int index = 0;
    for (int i = m_height-1; i >= 0; --i)
        for (int j = 0; j < m_width; ++j)
        {
            vec3 v = m_imageData[index++];
            vec3 c = clamp(v, 0.f, 1.f) * float(MaxRGB);
            Color colour(c.r, c.g, c.b);
            myImage.pixelColor(j, i, colour);
        }

    // try to write the image to the specified file
    try {
        myImage.write(imageFileName);
    }
    catch (Magick::Error &error) {
        cout << "Magick++ failed to write image " << imageFileName << endl;
        cout << "ERROR: " << error.what() << endl;
        return false;
    }
    return true;
#endif

    return false;
}

// --------------------------------------------------------------------------
