// ==========================================================================
// Line And Polygon Geometry
//
// Adapted from Sonny Chan's CPSC453 OpenGL Core Profile Boilerplate
// http://pages.cpsc.ucalgary.ca/~sonny.chan/cpsc453/resources/handouts/CPSC453-BoilerplateCode.zip
//
// Author:  Matthew Sembinelli, University of Calgary
// Date:    January 17, 2016
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>

using namespace std;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// --------------------------------------------------------------------------
// Enums & Globals
enum PolygonType
{
	SQUARES_DIAMONDS,
	PARAMETRIC_SPIRAL,
	SIERPINSKI_TRIANGLE
};

PolygonType poly_type = SQUARES_DIAMONDS;
GLuint poly_iter = 1;

vector<GLfloat> vertices;
vector<GLfloat> colours;

// --------------------------------------------------------------------------
// Structs

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
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader, string vertex, string fragment)
{
	// load shader source from files
	string vertexSource = LoadSource(vertex);
	string fragmentSource = LoadSource(fragment);
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

// --------------------------------------------------------------------------
// Functions to push primitives vertex and colour data onto vector

void AddVertex(GLfloat x, GLfloat y)
{
	vertices.push_back(x);
	vertices.push_back(y);
}

void AddColour(GLfloat r, GLfloat g, GLfloat b, vector<GLfloat> *vec = &colours)
{
	vec->push_back(r);
	vec->push_back(g);
	vec->push_back(b);
}

void AddLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    AddVertex(x1, y1);
	AddVertex(x2, y2);
}

void AddLineColour(GLfloat r1, GLfloat g1, GLfloat b1, GLfloat r2, GLfloat g2, GLfloat b2)
{
    AddColour(r1, g1, b1);
	AddColour(r2, g2, b2);
}

void AddTriangle(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3)
{
	AddVertex(x1, y1);
	AddVertex(x2, y2);
	AddVertex(x3, y3);
}

// --------------------------------------------------------------------------
// Functions to initialize scene data

bool InitializeSquaresAndDiamonds(MyGeometry *geometry)
{
	GLfloat sidelength = 1.80f;
	GLfloat vertex = sidelength/2.0f;
	
  //TODO add ability for nested squares to change colour at different levels
	AddLine(vertex, vertex, vertex, -vertex);
	AddLine(vertex, -vertex, -vertex, -vertex);
	AddLine(-vertex, -vertex, -vertex, vertex);
	AddLine(-vertex, vertex, vertex, vertex);
	
	AddLine(-vertex, 0.0, 0.0, vertex);
	AddLine(0.0, vertex, vertex, 0.0);
	AddLine(vertex, 0.0, 0.0, -vertex);
	AddLine(0.0, -vertex, -vertex, 0.0);
	
	for(int i = 0; i < 4; i++) { AddLineColour(1.0, 0.0, 0.0, 1.0, 0.0, 0.0); }
	for(int i = 0; i < 4; i++) { AddLineColour(0.0, 0.0, 1.0, 0.0, 0.0, 1.0); }

	geometry->elementCount = vertices.size()/2;

	return BindGeometryBuffers(geometry, &vertices, &colours);
}

bool InitializeParametricSpiral(MyGeometry *geometry, GLuint iter = 1)
{
	GLuint segments = 200 * iter;
	for(GLuint i = 0; i < segments; i++)
	{
		GLfloat theta = i*(iter*2.f*M_PI/(segments - 1));
		AddVertex(theta/(6.5*iter)*cos(theta), -theta/(6.5*iter)*sin(theta));
		AddColour(i*(1.0/segments), 1.0 - i*(1.0/segments), 0.0);
	}

	geometry->elementCount = vertices.size()/2;

	return BindGeometryBuffers(geometry, &vertices, &colours);
}

vector<GLfloat> triangle_colour;
GLuint triangle_colours_iter = 0;

void DivideTriangle(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3, GLuint iter)
{
	GLfloat new_x1, new_x2, new_x3, new_y1, new_y2, new_y3;
	if(iter > 0)
	{
		new_x1 = (x1 + x2)/2.0;
		new_y1 = (y1 + y2)/2.0;

		new_x2 = (x1 + x3)/2.0;
		new_y2 = (y1 + y3)/2.0;

		new_x3 = (x2 + x3)/2.0;
		new_y3 = (y2 + y3)/2.0;

		DivideTriangle(x1, y1, new_x1, new_y1, new_x2, new_y2, iter - 1);
		DivideTriangle(x3, y3, new_x3, new_y3, new_x2, new_y2, iter - 1);
		DivideTriangle(x2, y2, new_x3, new_y3, new_x1, new_y1, iter - 1);
	}
	else
	{
		AddTriangle(x1, y1, x2, y2, x3, y3);

		GLuint colour_iter = triangle_colours_iter*3;
		for(GLuint i = 0; i < 3; i++)
		    AddColour(triangle_colour.at(colour_iter), triangle_colour.at(colour_iter+1), triangle_colour.at(colour_iter+2));

		triangle_colours_iter++;
		if(triangle_colours_iter >= 3)
		    triangle_colours_iter = 0;
	}
}

bool InitializeSierpinskiTriangle(MyGeometry *geometry, GLuint iter = 1)
{
	GLfloat sidelength = 1.99f;
	GLfloat vertex = sidelength/2.0f;

	// Create base triangle
	AddTriangle(vertex, -vertex, 0.0, vertex*sin(M_PI/3.0), -vertex, -vertex);
	AddColour(0.0, 0.0, 0.0);
	AddColour(0.0, 0.0, 0.0);
	AddColour(0.0, 0.0, 0.0);

	// Create vector of colours that each recursion will cycle through
	triangle_colours_iter = 0;
	AddColour(0.0, 0.67, 0.66, &triangle_colour);
	AddColour(0.42, 0.0, 1.0, &triangle_colour);
	AddColour(0.0, 0.31, 0.94, &triangle_colour);

	// Divide starting triangle recursively
	DivideTriangle(vertex, -vertex, 0.0, vertex*sin(M_PI/3.0), -vertex, -vertex, iter);

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
	glClear(GL_COLOR_BUFFER_BIT);

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(shader->program);
	glBindVertexArray(geometry->vertexArray);

	switch(poly_type)
	{
	case SQUARES_DIAMONDS:
		glDrawArraysInstanced(GL_LINES, 0, geometry->elementCount, poly_iter);
		break;

	case PARAMETRIC_SPIRAL:
		glDrawArrays(GL_LINE_STRIP, 0, geometry->elementCount);
		break;

	case SIERPINSKI_TRIANGLE:
		glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);
		break;
	}

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
	if (action == GLFW_PRESS)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		switch(key)
		{
		case GLFW_KEY_F1:
			poly_type = SQUARES_DIAMONDS;
			poly_iter = 1;
			break;
		case GLFW_KEY_F2:
			poly_type = PARAMETRIC_SPIRAL;
			poly_iter = 1;
			break;
		case GLFW_KEY_F3:
			poly_type = SIERPINSKI_TRIANGLE;
			poly_iter = 0;
			break;
		case GLFW_KEY_UP:
			if(poly_iter <= 10)
			    poly_iter++;
			break;
		case GLFW_KEY_DOWN:
			if(poly_iter > 0)
			    poly_iter--;
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		default:
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
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(512, 512, "CPSC 453 Assignment 1 - Line And Polygon Geometry", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile shader programs
	MyShader shader[3];
	// call function to create and fill buffers with geometry data
	MyGeometry geometry[3];

	InitializeShaders(&shader[SQUARES_DIAMONDS], "vertex_sd.glsl", "fragment.glsl");
	InitializeSquaresAndDiamonds(&geometry[SQUARES_DIAMONDS]);
	InitializeShaders(&shader[PARAMETRIC_SPIRAL], "vertex.glsl", "fragment.glsl");
	InitializeParametricSpiral(&geometry[PARAMETRIC_SPIRAL]);
	InitializeShaders(&shader[SIERPINSKI_TRIANGLE], "vertex.glsl", "fragment.glsl");
	InitializeSierpinskiTriangle(&geometry[SIERPINSKI_TRIANGLE]);

	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{
		switch(poly_type)
		{
		case SQUARES_DIAMONDS:
			poly_type = SQUARES_DIAMONDS;
			break;

		case PARAMETRIC_SPIRAL:
			InitializeParametricSpiral(&geometry[PARAMETRIC_SPIRAL], poly_iter);
			poly_type = PARAMETRIC_SPIRAL;
			break;

		case SIERPINSKI_TRIANGLE:
			InitializeSierpinskiTriangle(&geometry[SIERPINSKI_TRIANGLE], poly_iter);
			poly_type = SIERPINSKI_TRIANGLE;
			break;
		}

		// Render scene based on user choice
		RenderScene(&geometry[poly_type], &shader[poly_type]);

		// scene is rendered to the back buffer, so swap to front for display
		glfwSwapBuffers(window);

		// sleep until next event before drawing again
		glfwWaitEvents();
	}

	// clean up allocated resources before exit
	for(GLuint i = 0; i < sizeof(geometry)/sizeof(geometry[0]); i++)
	{
		DestroyGeometry(&geometry[i]);
		DestroyShaders(&shader[i]);
	}

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

	ifstream input(filename.c_str());
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
