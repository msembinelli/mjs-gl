// ==========================================================================
// Line And Polygon Geometry
//
// Adapted from Sonny Chan's CPSC453 OpenGL Core Profile Boilerplate
// http://pages.cpsc.ucalgary.ca/~sonny.chan/cpsc453/resources/handouts/CPSC453-BoilerplateCode.zip
//
// Author:  Matthew Sembinelli, University of Calgary
// Date:    January 17, 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

using namespace std;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

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
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer[2];
    GLuint  colourBuffer[2];
    GLuint  vertexArray;
    GLsizei elementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer({0,0}), colourBuffer({0,0}), vertexArray(0), elementCount(0)
    {}
};

bool InitializeSquaresAndDiamonds(MyGeometry *geometry, GLfloat sidelength,  int iter)
{
	GLfloat vertex = sidelength/2.0f;
	GLfloat square_vertices[][2] =
	{
			{-vertex, -vertex},
			{-vertex,  vertex},
			{    0.0, -vertex},
			{-vertex,  vertex},
			{ vertex,  vertex},
			{    0.0, -vertex}

	};

	GLfloat colours1[][3] =
	{
	        { 1.0, 0.0, 0.0 },
	        { 1.0, 0.0, 0.0 },
	        { 1.0, 0.0, 0.0 },
	        { 1.0, 0.0, 0.0 },
	        { 1.0, 0.0, 0.0 },
	        { 1.0, 0.0, 0.0 }

	};

	GLfloat colours2[][3] =
	{
	        { 0.0, 1.0, 0.0 },
	        { 0.0, 1.0, 0.0 },
	        { 0.0, 1.0, 0.0 },
	        { 0.0, 1.0, 0.0 },
	        { 0.0, 1.0, 0.0 },
	        { 0.0, 1.0, 0.0 }

	};

	GLfloat square_vertices_trans[6][2];

	for(int i = 0; i < 6; i++){
		glm::vec3 v = glm::vec3(square_vertices[i][0], square_vertices[i][1], 1.0f);
		glm::vec3 v_trans;
		v_trans = glm::rotate(v, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 mat(1.0f);
		mat = glm::scale(mat, glm::vec3(glm::sqrt(2*glm::pow(vertex, 2))));
		glm::vec4 n(v_trans, 0.0f);
		mat *= n;
		square_vertices_trans[i][0] = glm::value_ptr(mat)[0];
		square_vertices_trans[i][1] = glm::value_ptr(mat)[5];
	}

    geometry->elementCount = 12;

    // create an array buffer object for storing our vertices
    glGenBuffers(2, geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices_trans), square_vertices_trans, GL_STATIC_DRAW);

    // create another one for storing our colours
    glGenBuffers(2, geometry->colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours1), colours1, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours2), colours2, GL_STATIC_DRAW);

    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);

    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // associate the colour array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer[0]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer[1]);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);

    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	return !CheckGLErrors();
}

bool InitializeParametricSpiral(MyGeometry *geometry,  int iter){ return !CheckGLErrors(); }

bool InitializeSierpinskiTriangle(MyGeometry *geometry,  int iter){ return !CheckGLErrors(); }

// create buffers and fill with geometry data, returning true if successful
/*bool InitializeGeometry(MyGeometry *geometry, int iter)
{
    //Draw square consisting of two triangles
    const GLfloat vertices[][2] = {
        { -0.5,  0.5 },
        {  0.0,  0.5 },
        { -0.5,  0.0 },
        {  0.0,  0.5 },
        {  0.5,  0.0 },
        {  0.5,  0.5 },
		{  0.5,  0.0 },
		{  0.5, -0.5 },
		{  0.0, -0.5 },
		{ -0.5, -0.5 },
		{ -0.5,  0.0 },
		{  0.0, -0.5 },
		{ -0.5,  0.0 },
		{  0.0,  0.5 },
		{  0.5,  0.0 }
    };

    const GLfloat colours[][3] = {
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 0.0, 1.0, 0.0 }
    };
    geometry->elementCount = 15;

    // these vertex attribute indices correspond to those specified for the
    // input variables in the vertex shader
    const GLuint VERTEX_INDEX = 0;
    const GLuint COLOUR_INDEX = 1;

    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // create another one for storing our colours
    glGenBuffers(1, &geometry->colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);

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

    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}*/

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(2, geometry->vertexBuffer);
    glDeleteBuffers(2, geometry->colourBuffer);
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
    glBindVertexArray(geometry->vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

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

enum PolygonType
{
	SQUARES_DIAMONDS,
	PARAMETRIC_SPIRAL,
	SIERPINSKI_TRIANGLE
};

PolygonType poly_type = SQUARES_DIAMONDS;
int poly_iter = 1;

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
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
    		poly_iter = 1;
    		break;
    	case GLFW_KEY_1:
    		poly_iter = 1;
    		break;
    	case GLFW_KEY_2:
    		poly_iter = 2;
    		break;
    	case GLFW_KEY_3:
    		poly_iter = 3;
    		break;
    	case GLFW_KEY_4:
    		poly_iter = 4;
    		break;
    	case GLFW_KEY_5:
    		poly_iter = 5;
    		break;
    	case GLFW_KEY_6:
    		poly_iter = 6;
    		break;
    	case GLFW_KEY_ESCAPE:
    		glfwSetWindowShouldClose(window, GL_TRUE);
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
    MyShader sd_shader, ps_shader, st_shader;
    /*if (!InitializeShaders(&sd_shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }*/

    // call function to create and fill buffers with geometry data
    MyGeometry sd_geometry, ps_geometry, st_geometry;
    /*if (!InitializeGeometry(&sd_geometry, 1))
        cout << "Program failed to initialize geometry!" << endl;*/

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
    	switch(poly_type)
    	{
    	case SQUARES_DIAMONDS:
    		InitializeShaders(&sd_shader);
    		InitializeSquaresAndDiamonds(&sd_geometry, 1.0, poly_iter);
    		RenderScene(&sd_geometry, &sd_shader);
    		break;
    	case PARAMETRIC_SPIRAL:
    		InitializeParametricSpiral(&ps_geometry, poly_iter);
    		//RenderScene(&ps_geometry, &shader);
    		break;
    	case SIERPINSKI_TRIANGLE:
    		InitializeSierpinskiTriangle(&st_geometry, poly_iter);
    		//RenderScene(&st_geometry, &shader);
    		break;
    	}

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwWaitEvents();
    }

    // clean up allocated resources before exit
    DestroyGeometry(&sd_geometry);
    DestroyGeometry(&ps_geometry);
    DestroyGeometry(&st_geometry);
    DestroyShaders(&sd_shader);
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
