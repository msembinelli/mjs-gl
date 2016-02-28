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
#include <utility>
#include <vector>
#include <cmath>

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include "GlyphExtractor.h"

using namespace std;

// --------------------------------------------------------------------------
// Enum used for switching scenes

enum Scene
{
    BEZIER,
	FONTS,
	SCROLLING_TEXT,
	SCENE_MAX
};

// --------------------------------------------------------------------------
// Enum used for switching bezier type

enum BezierScene
{
	BEZIER_QUADRATIC,
	BEZIER_CUBIC,
	BEZIER_MAX
};

// --------------------------------------------------------------------------
// Enum used for switching font type

//TODO add flag to show/ hide control points/polygons

enum FontScene
{
	FONT_LORA,
	FONT_SOURCE_SANS_PRO,
	FONT_CUSTOM, //TODO pick font
	FONT_MAX
};

// --------------------------------------------------------------------------
// Enum used for switching scroll font type

enum ScrollScene
{
	SCROLL_ALEX_BRUSH,
	SCROLL_INCONSOLATA,
	SCROLL_CUSTOM, //TODO pick font
	SCROLL_MAX
};

// --------------------------------------------------------------------------
// Globals

Scene scene = BEZIER;
BezierScene bezier_scene = BEZIER_QUADRATIC;
FontScene font_scene = FONT_LORA;
ScrollScene scroll_scene = SCROLL_ALEX_BRUSH;

vector<GLfloat> vertices;
vector<GLfloat> colours;

// --------------------------------------------------------------------------
// Universal geometry functions

void NormalizeVertices()
{
	vector<GLfloat>::const_iterator it_max, it_min;
	it_max = max_element(vertices.begin(), vertices.end());
	it_min = min_element(vertices.begin(), vertices.end());
	GLfloat scale = fabs(*it_max) > fabs(*it_min) ? fabs(*it_max) : fabs(*it_min);
	if(scale != 0)
	    for(GLuint i = 0; i < vertices.size(); i++){ vertices[i] /= scale; }

    //TODO CENTER IMAGE, SCALE TO FIT WINDOW?

}

void AddControlPoint(GLfloat x, GLfloat y)
{
	vertices.push_back(x);
	vertices.push_back(y);
}

void AddColour(GLfloat r, GLfloat g, GLfloat b)
{
	colours.push_back(r);
	colours.push_back(g);
	colours.push_back(b);
}

// --------------------------------------------------------------------------
// Bezier functions

void CubicBezierVertices(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4)
{
	AddControlPoint(x1, y1);
	AddColour(1.0, 0.0, 0.0);
	AddControlPoint(x2, y2);
	AddColour(0.0, 0.0, 1.0);
	AddControlPoint(x3, y3);
	AddColour(0.0, 0.0, 1.0);
	AddControlPoint(x4, y4);
	AddColour(1.0, 0.0, 0.0);
}

void QuadraticBezierVertices(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3)
{
	CubicBezierVertices(x1, y1, x2, y2, x3, y3, x3, y3);
}

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string shaderPath = "shaders/";
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint controlShader, GLuint evalShader);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
    // OpenGL names for vertex and fragment shaders, shader program
    GLuint  vertex;
    GLuint  fragment;
    GLuint  tessEval;
    GLuint  tessControl;
    GLuint  tessProgram;
    GLuint  controlProgram;

    // initialize shader and program names to zero (OpenGL reserved value)
    MyShader() : vertex(0), fragment(0), tessEval(0), tessControl(0), tessProgram(0), controlProgram(0)
    {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
    // load shader source from files
    string vertexSource = LoadSource(shaderPath + "vertex.glsl");
    string fragmentSource = LoadSource(shaderPath + "fragment.glsl");
    string tessEvalSource = LoadSource(shaderPath + "tess_eval.glsl");
    string tessControlSource = LoadSource(shaderPath + "tess_control.glsl");

    if (vertexSource.empty() || fragmentSource.empty() || tessEvalSource.empty() || tessControlSource.empty()) return false;

    // compile shader source into shader objects
    shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    shader->tessEval = CompileShader(GL_TESS_EVALUATION_SHADER, tessEvalSource);
    shader->tessControl = CompileShader(GL_TESS_CONTROL_SHADER, tessControlSource);

    // link shader program
    shader->tessProgram = LinkProgram(shader->vertex, shader->fragment, shader->tessControl, shader->tessEval);
    shader->controlProgram = LinkProgram(shader->vertex, shader->fragment);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
    // unbind any shader programs and destroy shader objects
    glUseProgram(0);
    glDeleteProgram(shader->tessProgram);
    glDeleteProgram(shader->controlProgram);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
    glDeleteShader(shader->tessEval);
    glDeleteShader(shader->tessControl);
}

// --------------------------------------------------------------------------
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
	glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	// associate the colour array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool InitializeCubicBezier(MyGeometry *geometry)
{
	CubicBezierVertices(1, 1, 4, 0, 6, 2, 9, 1);
    CubicBezierVertices(8, 2, 0, 8, 0, -2, 9, 4);
    CubicBezierVertices(5, 3, 3, 2, 3, 3, 5, 2);
    CubicBezierVertices(3, 2.2, 3.5, 2.7, 3.5, 3.3, 3, 3.8);
    CubicBezierVertices(2.8, 3.5, 2.4, 3.8, 2.4, 3.2, 2.8, 3.5);
    NormalizeVertices();

    geometry->elementCount = vertices.size()/2;
	return BindGeometryBuffers(geometry, &vertices, &colours);
}

bool InitializeQuadraticBezier(MyGeometry *geometry)
{
	QuadraticBezierVertices(1.0, 1.0, 2.0, -1.0, 0, -1.0);
	QuadraticBezierVertices(0, -1, -2, -1, -1, 1);
	QuadraticBezierVertices(-1, 1, 0, 1, 1, 1);
	QuadraticBezierVertices(1.2, 0.5, 2.5, 1.0, 1.2, -0.4);
    NormalizeVertices();

    geometry->elementCount = vertices.size()/2;
	return BindGeometryBuffers(geometry, &vertices, &colours);
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

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader)
{
    // clear screen to a dark grey colour
    //glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->tessProgram);
    glUniform1ui(glGetUniformLocation(shader->tessProgram, "bezier_scene"), bezier_scene);
    glUniform1i(glGetUniformLocation(shader->tessProgram, "control_points"), false);
    glUniform1i(glGetUniformLocation(shader->tessProgram, "control_lines"), false);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glBindVertexArray(geometry->vertexArray);
    glDrawArrays(GL_PATCHES, 0, geometry->elementCount);

    // Draw control points and polygons for bezier curves
    glUseProgram(shader->controlProgram);
    glUniform1i(glGetUniformLocation(shader->controlProgram, "control_lines"), true);
    glUniform1i(glGetUniformLocation(shader->controlProgram, "control_points"), false);
    glDrawArrays(GL_LINE_STRIP, 0, geometry->elementCount);

    glUniform1i(glGetUniformLocation(shader->controlProgram, "control_lines"), false);
    glUniform1i(glGetUniformLocation(shader->controlProgram, "control_points"), true);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, geometry->elementCount);


    glDisable(GL_PROGRAM_POINT_SIZE);
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

	if (action == GLFW_PRESS)
	{
		switch(key)
		{
		case GLFW_KEY_F1:
			scene = BEZIER;
			break;
		case GLFW_KEY_F2:
			scene = FONTS;
			break;
		case GLFW_KEY_F3:
			scene = SCROLLING_TEXT;
			break;
		case GLFW_KEY_UP:
			switch(scene)
			{
			case BEZIER:
				if(bezier_scene < BEZIER_MAX - 1)
				    bezier_scene = BezierScene((GLuint)bezier_scene + 1);
				break;
			case FONTS:
				if(font_scene < FONT_MAX - 1)
				    font_scene = FontScene((GLuint)font_scene + 1);
				break;
			case SCROLLING_TEXT:
				if(scroll_scene < SCROLL_MAX - 1)
				    scroll_scene = ScrollScene((GLuint)scroll_scene + 1);
				break;
			}
			break;
		case GLFW_KEY_DOWN:
		    switch(scene)
		    {
		    case BEZIER:
				if(bezier_scene > 0)
		    	    bezier_scene = BezierScene((GLuint)bezier_scene - 1);
		    	break;
		    case FONTS:
				if(font_scene > 0)
		    	    font_scene = FontScene((GLuint)font_scene - 1);
		    	break;
		    case SCROLLING_TEXT:
				if(scroll_scene > 0)
		    	    scroll_scene = ScrollScene((GLuint)scroll_scene - 1);
		    	break;
		    }
		    break;
		}
	}
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(512, 512, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);

    //Load Inconsolata font and print some info
  	GlyphExtractor* ge = new GlyphExtractor();
    if(!ge->LoadFontFile("CPSC453-A3-Fonts/Inconsolata.otf"))
      cout << "font Inconsolata.otf loaded" << endl;
    MyGlyph aGlyph = ge->ExtractGlyph('a');

    // query and print out information about our OpenGL environment
    QueryGLVersion();

    // call function to load and compile shader programs
    MyShader shader;
    if (!InitializeShaders(&shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }
    // call function to create and fill buffers with geometry data
    MyGeometry bezier_geometry[BEZIER_MAX];
    if (!InitializeQuadraticBezier(&bezier_geometry[BEZIER_QUADRATIC]))
        cout << "Program failed to intialize geometry!" << endl;

    vertices.clear();
    colours.clear();

    if (!InitializeCubicBezier(&bezier_geometry[BEZIER_CUBIC]))
        cout << "Program failed to intialize geometry!" << endl;

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        // call function to draw our scene
        RenderScene(&bezier_geometry[bezier_scene], &shader);

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwWaitEvents();
    }

    // clean up allocated resources before exit
    DestroyGeometry(&bezier_geometry[bezier_scene]);
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
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint controlShader, GLuint evalShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);
    if (controlShader)  glAttachShader(programObject, controlShader);
    if (evalShader)  glAttachShader(programObject, evalShader);

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
