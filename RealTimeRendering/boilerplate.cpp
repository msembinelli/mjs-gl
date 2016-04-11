// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if defined(_WIN32)
#include <windows.h>
#include <GL/glew.h>
#define M_PI 3.14159
#else
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <math.h>
#endif

// specify that we want the OpenGL core profile before including GLFW headers

#include <GLFW/glfw3.h>
#include <Magick++.h>

#include "ImageReader.h"

using namespace std;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// Add uniform locations =============
GLuint projUniform;
GLuint viewUniform;
GLuint modelUniform;

GLfloat angle = 0;
GLfloat lastTime = 0;
GLfloat rotation_speed = 0.80;
vector<GLfloat> vertices;
vector<GLfloat> colours;


// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
    // OpenGL names for vertex and fragment shaders, shader program
    GLuint  vertex;
    GLuint  fragment;
    GLuint  program;

    // initialize shader and program names to zero (OpenGL reserved value)
    MyShader() : vertex(0), fragment(0), program(0)
    {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
    // load shader source from files
    string vertexSource = LoadSource("vertex.glsl");
    string fragmentSource = LoadSource("fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty()) return false;

    // compile shader source into shader objects
    shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // link shader program
    shader->program = LinkProgram(shader->vertex, shader->fragment);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
    // unbind any shader programs and destroy shader objects
    glUseProgram(0);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Add method to set transform uniforms


// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer;
    GLuint  colourBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
    {}
};

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

bool BindGeometryBuffers(MyGeometry *geometry, vector<GLfloat> *vertex_vec, vector<GLfloat> *colour_vec)
{
  GLuint VERTEX_INDEX = 0;
  GLuint COLOUR_INDEX = 1;

  // create an array buffer object for storing our vertices
  glGenBuffers(1, &geometry->vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertex_vec->size()*sizeof(GLfloat), vertex_vec->data(), GL_STATIC_DRAW);

  // create another one for storing our colours
  glGenBuffers(1, &geometry->colourBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
  glBufferData(GL_ARRAY_BUFFER, colour_vec->size()*sizeof(GLfloat), colour_vec->data(), GL_STATIC_DRAW);

  // create a vertex array object encapsulating all our vertex attributes
  glGenVertexArrays(1, &geometry->vertexArray);
  glBindVertexArray(geometry->vertexArray);

  // associate the position array with the vertex array object
  glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
  glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(VERTEX_INDEX);

  // associate the colour array with the vertex array object
  glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
  glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(COLOUR_INDEX);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return !CheckGLErrors();
}

// --------------------------------------------------------------------------
// Functions to push primitives vertex and colour data onto vector

void AddVertex(GLfloat x, GLfloat y, GLfloat z)
{
  vertices.push_back(x);
  vertices.push_back(y);
  vertices.push_back(z);
}

void AddColour(GLfloat r, GLfloat g, GLfloat b)
{
  colours.push_back(r);
  colours.push_back(g);
  colours.push_back(b);
}

void AddLine(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2)
{
  AddVertex(x1, y1, z1);
  AddVertex(x2, y2, z2);
}

void AddLineColour(GLfloat r1, GLfloat g1, GLfloat b1, GLfloat r2, GLfloat g2, GLfloat b2)
{
  AddColour(r1, g1, b1);
  AddColour(r2, g2, b2);
}

void AddTriangle(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2, GLfloat x3, GLfloat y3, GLfloat z3)
{
  AddVertex(x1, y1, z1);
  AddVertex(x2, y2, z2);
  AddVertex(x3, y3, z3);
}

void AddTriangleColour(GLfloat r1, GLfloat g1, GLfloat b1, GLfloat r2, GLfloat g2, GLfloat b2, GLfloat r3, GLfloat g3, GLfloat b3)
{
  AddColour(r1, g1, b1);
  AddColour(r2, g2, b2);
  AddColour(r3, g3, b3);
}

bool InitializeSphere(MyGeometry *geometry)
{
  GLfloat step_size = (M_PI / 36.0);

  for (GLfloat altitude = 0.0; altitude <= M_PI; altitude += (step_size))
  {
      for (GLfloat azimuth = 0.0; azimuth <= (2 * M_PI); azimuth += (step_size))
      {
          GLfloat x1 = glm::sin(altitude)*glm::cos(azimuth);
          GLfloat y1 = glm::sin(altitude)*glm::sin(azimuth);
          GLfloat z1 = glm::cos(altitude);

          GLfloat x2 = glm::sin(altitude)*glm::cos(azimuth + (step_size));
          GLfloat y2 = glm::sin(altitude)*glm::sin(azimuth + (step_size));
          GLfloat z2 = glm::cos(altitude);

          GLfloat x3 = glm::sin(altitude + (step_size))*glm::cos(azimuth + (step_size));
          GLfloat y3 = glm::sin(altitude + (step_size))*glm::sin(azimuth + (step_size));
          GLfloat z3 = glm::cos(altitude + (step_size));

          AddTriangle(x1, y1, z1, x2, y2, z2, x3, y3, z3);
          AddTriangleColour(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);

          x2 = glm::sin(altitude + (step_size))*glm::cos(azimuth);
          y2 = glm::sin(altitude + (step_size))*glm::sin(azimuth);
          z2 = glm::cos(altitude + (step_size));

          AddTriangle(x1, y1, z1, x2, y2, z2, x3, y3, z3);
          AddTriangleColour(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);

      }
  }

  geometry->elementCount = vertices.size() / 3;

  return BindGeometryBuffers(geometry, &vertices, &colours);
}

void GetRotationRate()
{
  GLfloat currentTime;

  currentTime = glfwGetTime();

  angle += rotation_speed / (1 / GLfloat(currentTime - lastTime));
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(1, &geometry->vertexBuffer);
    glDeleteBuffers(1, &geometry->colourBuffer);
}

void setTransformationUniform(GLuint uniform, glm::mat4 mat)
{
    glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(mat));
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader)
{
    // clear screen to a dark grey colour
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->program);

    // Update model uniform and update angle  ======================
    setTransformationUniform(modelUniform, glm::rotate(glm::translate(
                                           glm::rotate(angle, glm::vec3(0,1,0)),
                                           glm::vec3(1,0,0)), angle *2.f, glm::vec3(0,1,0)));

    glBindVertexArray(geometry->vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

    vertices.clear();
    colours.clear();

    // reset state to default (no shader or geometry bound)
    glBindVertexArray(0);
    glUseProgram(0);

    // check for an report any OpenGL errors
    CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    Magick::InitializeMagick(NULL);

    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
#if !defined(_WIN32)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    int width = 512, height = 512;
    window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);


#if defined(_WIN32)
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
      //Problem: glewInit failed, something is seriously wrong.
      cout << "glewInit failed, aborting." << endl;
    }
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR){}
#endif

    // query and print out information about our OpenGL environment
    QueryGLVersion();

    // call function to load and compile shader programs
    MyShader shader;
    if (!InitializeShaders(&shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }

    // Setup transformation uniforms ==================
    glUseProgram(shader.program);
    projUniform = glGetUniformLocation(shader.program, "proj");
    viewUniform = glGetUniformLocation(shader.program, "view");
    modelUniform = glGetUniformLocation(shader.program, "model");
    
    glm::mat4 I(1.f);
    
    setTransformationUniform(projUniform, glm::perspective(45.f, float(width)/float(height), 0.1f, 1000.f));
    setTransformationUniform(viewUniform, glm::lookAt(glm::vec3(0,1,2), glm::vec3(0,0,-1), glm::vec3(0,1,0)));
    setTransformationUniform(modelUniform, I);

    // call function to create and fill buffers with geometry data
    MyGeometry geometry;
    /*if (!InitializeGeometry(&geometry))
        cout << "Program failed to intialize geometry!" << endl;*/
    if (!InitializeSphere(&geometry))
      cout << "Program failed to intialize geometry!" << endl;

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        lastTime = glfwGetTime();

        // call function to draw our scene
        RenderScene(&geometry, &shader);

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();

        GetRotationRate();
    }

    // clean up allocated resources before exit
    DestroyGeometry(&geometry);
    DestroyShaders(&shader);
    glfwDestroyWindow(window);
    glfwTerminate();

    cout << "Goodbye!" << endl;
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename);
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}


// ==========================================================================
