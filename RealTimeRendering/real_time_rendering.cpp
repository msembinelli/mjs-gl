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
#include <glm/gtx/compatibility.hpp>
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

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define MAX_CAMERA_RADIUS 100.0
#define MIN_CAMERA_RADIUS 10.0
#define SPHERE_ANGLE_SIZE (M_PI / 12.0)

GLfloat angle = 0;
GLfloat view_azimuth = 0.0;
GLfloat view_altitude = M_PI / 3.0;
GLfloat view_radius = 50.0;
GLfloat lastTime = 0;
GLfloat rotation_speed = 0.80;
vector<GLfloat> vertices;
vector<GLfloat> colours;
vector<GLfloat> uv_coords;
int mouse_button = GLFW_KEY_UNKNOWN;
bool mouse_press = false;

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
    GLuint  textureCoordBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), colourBuffer(0), textureCoordBuffer(0), vertexArray(0), elementCount(0)
    {}
};

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

bool BindGeometryBuffers(MyGeometry *geometry, vector<GLfloat> *vertex_vec, vector<GLfloat> *colour_vec, vector<GLfloat> *texture_vec)
{
  GLuint VERTEX_INDEX = 0;
  GLuint COLOUR_INDEX = 1;
  GLuint TEXTURE_INDEX = 2;

  // create an array buffer object for storing our vertices
  glGenBuffers(1, &geometry->vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertex_vec->size()*sizeof(GLfloat), vertex_vec->data(), GL_STATIC_DRAW);

  // create another one for storing our colours
  glGenBuffers(1, &geometry->colourBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
  glBufferData(GL_ARRAY_BUFFER, colour_vec->size()*sizeof(GLfloat), colour_vec->data(), GL_STATIC_DRAW);

  // create another one for storing our colours
  glGenBuffers(1, &geometry->textureCoordBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
  glBufferData(GL_ARRAY_BUFFER, texture_vec->size()*sizeof(GLfloat), texture_vec->data(), GL_STATIC_DRAW);

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

  // Set up vertex attribute info for textures
  glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
  glVertexAttribPointer(TEXTURE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(TEXTURE_INDEX);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  vertex_vec->clear();
  colour_vec->clear();
  texture_vec->clear();

  return !CheckGLErrors();
}

// --------------------------------------------------------------------------
// CelestialBody Class

class CelestialBody
{
public:
  CelestialBody(glm::vec3 origin_, GLfloat revolution_rate_, GLfloat rotational_rate_, MyGeometry *geometry_, MyTexture *texture_, CelestialBody *satellite_ = NULL);
  glm::mat4 GenerateTransformationMatrix(glm::mat4 previous_model);
  CelestialBody* GetChildSatellite();
  MyGeometry* GetGeometry();
  MyTexture* GetTexture();
private:
  glm::vec3 origin; //relative to parent body
  GLfloat revolution_rate; //relative to parent body
  GLfloat rotational_rate; //relative to parent body
  CelestialBody *satellite;
  MyGeometry *geometry;
  MyTexture *texture;
};

CelestialBody::CelestialBody(glm::vec3 origin_, GLfloat revolution_rate_, GLfloat rotational_rate_, MyGeometry *geometry_, MyTexture *texture_, CelestialBody *satellite_)
{
  origin = origin_;
  revolution_rate = revolution_rate_;
  rotational_rate = rotational_rate_;
  geometry = geometry_;
  texture = texture_;
  satellite = satellite_;
}
glm::mat4 CelestialBody::GenerateTransformationMatrix(glm::mat4 previous_model)
{
  glm::mat4 model;
  model = glm::rotate(previous_model, rotational_rate * angle, glm::vec3(0, 1, 0));
  model = glm::translate(model, origin);
  model = glm::rotate(model, revolution_rate * angle, glm::vec3(0, 1, 0));
;
  return model;
}
MyGeometry* CelestialBody::GetGeometry()
{
  return geometry;
}
MyTexture* CelestialBody::GetTexture()
{
  return texture;
}
CelestialBody* CelestialBody::GetChildSatellite()
{
  return satellite;
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

void AddUVCoordinate(glm::vec3 vector)
{
  glm::vec3 d = glm::normalize(vector);
  GLfloat u = 0.5 * (atan2(d.z, d.x) / (M_PI)+1.0);
  GLfloat v = 0.5 + asin(d.y)/(M_PI);
  uv_coords.push_back(u);
  uv_coords.push_back(v);

}

glm::vec3 SphericalToCartesian(GLfloat altitude, GLfloat azimuth, GLfloat radius = 1.0)
{
  glm::vec3 return_vec =  glm::vec3(radius*glm::sin(altitude)*glm::cos(azimuth), radius*glm::cos(altitude), radius*glm::sin(altitude)*glm::sin(azimuth));
  return return_vec;
}

bool InitializeSphere(MyGeometry *geometry, GLfloat radius)
{
  GLfloat step_size = SPHERE_ANGLE_SIZE;

  for (GLfloat altitude = 0.0; altitude <= (M_PI - SPHERE_ANGLE_SIZE); altitude += (step_size))
  {
    for (GLfloat azimuth = 0.0; azimuth <= (2 * M_PI); azimuth += (step_size))
    {
      glm::vec3 vertex_1, vertex_2, vertex_3;
      if (altitude == 0.0)
      {
        vertex_1 = SphericalToCartesian(altitude, azimuth, radius);
        vertex_2 = SphericalToCartesian(altitude + step_size, azimuth, radius);
        vertex_3 = SphericalToCartesian(altitude + step_size, azimuth + step_size, radius);
        AddUVCoordinate(vertex_3);
        AddUVCoordinate(vertex_2);
        AddUVCoordinate(vertex_1);
        AddTriangle(vertex_3.x, vertex_3.y, vertex_3.z, vertex_2.x, vertex_2.y, vertex_2.z, vertex_1.x, vertex_1.y, vertex_1.z);
      }
      else if (altitude == (M_PI - SPHERE_ANGLE_SIZE))
      {
        vertex_1 = SphericalToCartesian(altitude, azimuth, radius);
        vertex_2 = SphericalToCartesian(altitude, azimuth + step_size, radius);
        vertex_3 = SphericalToCartesian(altitude + step_size, azimuth, radius);
        AddUVCoordinate(vertex_1);
        AddUVCoordinate(vertex_2);
        AddUVCoordinate(vertex_3);
        AddTriangle(vertex_1.x, vertex_1.y, vertex_1.z, vertex_2.x, vertex_2.y, vertex_2.z, vertex_3.x, vertex_3.y, vertex_3.z);
      }
      else
      {
        vertex_1 = SphericalToCartesian(altitude, azimuth, radius);
        vertex_2 = SphericalToCartesian(altitude, azimuth + step_size, radius);
        vertex_3 = SphericalToCartesian(altitude + step_size, azimuth + step_size, radius);
        AddUVCoordinate(vertex_1);
        AddUVCoordinate(vertex_2);
        AddUVCoordinate(vertex_3);
        AddTriangle(vertex_1.x, vertex_1.y, vertex_1.z, vertex_2.x, vertex_2.y, vertex_2.z, vertex_3.x, vertex_3.y, vertex_3.z);
        vertex_2 = SphericalToCartesian(altitude + step_size, azimuth, radius);
        AddUVCoordinate(vertex_3);
        AddUVCoordinate(vertex_2);
        AddUVCoordinate(vertex_1);
        AddTriangle(vertex_3.x, vertex_3.y, vertex_3.z, vertex_2.x, vertex_2.y, vertex_2.z, vertex_1.x, vertex_1.y, vertex_1.z);
      }
    }
  }

  geometry->elementCount = vertices.size() / 3;

  return BindGeometryBuffers(geometry, &vertices, &colours, &uv_coords);
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

void RenderScene(MyShader *shader, CelestialBody *body_graph)
{
    // clear screen to a dark grey colour
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->program);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    CelestialBody *body = body_graph;
    glm::mat4 last_model(1.f);

    while (body != NULL)
    {
      glBindVertexArray(body->GetGeometry()->vertexArray);
      glBindTexture(GL_TEXTURE_2D, body->GetTexture()->textureName);
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glm::mat4 model = body->GenerateTransformationMatrix(last_model);
      setTransformationUniform(modelUniform, model);
      last_model = model;
      glDrawArrays(GL_TRIANGLES, 0, body->GetGeometry()->elementCount);
      body = body->GetChildSatellite();
    }

    // reset state to default (no shader or geometry bound)
    glBindTexture(GL_TEXTURE_2D, 0);
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

    if (action == GLFW_PRESS)
    {
      switch (key)
      {
      case GLFW_KEY_UP:
        rotation_speed += 0.05;
        break;
      case GLFW_KEY_DOWN:
        rotation_speed -= 0.05;
        break;
      }
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
      mouse_button = button;
      mouse_press = true;
    }
    else if (action == GLFW_RELEASE)
    {
      mouse_press = false;
      mouse_button = GLFW_KEY_UNKNOWN;
    }
}

void MouseMoveCallback(GLFWwindow* window, double x, double y)
{
  if (mouse_press)
  {
    if (mouse_button == GLFW_MOUSE_BUTTON_1)
    {
      GLfloat normalized_x = x / float(WINDOW_HEIGHT);
      if (normalized_x <= 1.0 && normalized_x >= 0.01)
      {
        view_azimuth = normalized_x * (2 * M_PI);
      }

    }
    if (mouse_button == GLFW_MOUSE_BUTTON_2)
    {
      GLfloat normalized_y = y / float(WINDOW_WIDTH);
      if (normalized_y <= 1.0 && normalized_y >= 0.01)
      {
        view_altitude = normalized_y * (M_PI);
      }
    }
  }
}

void MouseScrollwheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset == 1 && view_radius > MIN_CAMERA_RADIUS)
      view_radius -= 1.0;
    else if (yoffset == -1 && view_radius <= MAX_CAMERA_RADIUS)
      view_radius += 1.0;
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
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, MouseScrollwheelCallback);
    glfwSetCursorPosCallback(window, MouseMoveCallback);
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
    
    setTransformationUniform(projUniform, glm::perspective(45.f, float(WINDOW_WIDTH)/float(WINDOW_HEIGHT), 0.01f, 1000.f));
    setTransformationUniform(viewUniform, glm::lookAt(SphericalToCartesian(view_altitude, view_azimuth, view_radius), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
    setTransformationUniform(modelUniform, I);

    MyTexture sun_texture, earth_texture, moon_texture;

    if (!InitializeTexture(&sun_texture, "sun.jpg"))
      cout << "Failed to load sun_texture!" << endl;

    if (!InitializeTexture(&earth_texture, "earth.jpg"))
      cout << "Failed to load earth_texture!" << endl;

    if (!InitializeTexture(&moon_texture, "moon.jpg"))
      cout << "Failed to load moon_texture!" << endl;

    // call function to create and fill buffers with geometry data
    MyGeometry sun_geometry, earth_geometry, moon_geometry;
    if (!InitializeSphere(&sun_geometry, log(109)))
      cout << "Program failed to intialize sun_geometry!" << endl;
    if (!InitializeSphere(&earth_geometry, 1.0))
      cout << "Program failed to intialize earth_geometry!" << endl;
    if (!InitializeSphere(&moon_geometry, 0.27))
      cout << "Program failed to intialize moon_geometry!" << endl;

    CelestialBody moon = CelestialBody(glm::vec3(2.57, 0.0, 0.0), (0.073), (27), &moon_geometry, &moon_texture);
    CelestialBody earth = CelestialBody(glm::vec3(15, 0.0, 0.0), (1.0), 100*(0.0027), &earth_geometry, &earth_texture, &moon);
    CelestialBody sun = CelestialBody(glm::vec3(0.0), 0.0, 0.0, &sun_geometry, &sun_texture, &earth);

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(shader.program);
        lastTime = glfwGetTime();

        setTransformationUniform(viewUniform, glm::lookAt(SphericalToCartesian(view_altitude, view_azimuth, view_radius), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

        // call function to draw our scene
        RenderScene(&shader, &sun);

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();

        GetRotationRate();
    }

    // clean up allocated resources before exit
    DestroyGeometry(&sun_geometry);
    DestroyGeometry(&earth_geometry);
    DestroyGeometry(&moon_geometry);
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
