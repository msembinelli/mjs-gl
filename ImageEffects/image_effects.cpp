// ==========================================================================
// Image Effects
//
// Author:  Matthew Sembinelli, University of Calgary
// Date:    February 13, 2016
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <math.h>

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#include <GLFW/glfw3.h>
#include <Magick++.h>
#include <glm/glm.hpp>

using namespace std;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);


// ==========================================================================

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

// Enum to keep track of current image
enum ImageType
{
	MANDRILL,
	UCLOGO,
	AERIAL,
	THIRSK,
	PATTERN,
	CAPSTONE,
	IMAGETYPE_MAX
};

ImageType image_type = MANDRILL;

enum FilterType
{
    NO_FILTER,
	VERTICAL_SOBEL,
	HORIZONTAL_SOBEL,
	UNSHARP_MASK,
	GAUSSIAN,
	FILTERTYPE_MAX
};

FilterType filter_type = NO_FILTER;

GLint kernels[4][9] = {
		{0, 0, 0, 0, 1, 0, 0, 0, 0},
		{1, 0, -1, 2, 0, -2, 1, 0, -1},
		{1, 2, 1, 0, 0, 0, -1, -2, -1},
		{0, -1, 0, -1, 5, -1, 0, -1, 0}
};

enum GaussianType
{
    NO_GAUSSIAN = 0,
	GAUSSIAN_3X3 = 3,
	GAUSSIAN_5X5 = 5,
	GAUSSIAN_7X7 = 7,
	GAUSSIAN_MAX
};

GaussianType gaussian_type = NO_GAUSSIAN;

struct Transformation
{
    GLfloat  x;
    GLfloat  y;
    GLfloat  scale;
    GLfloat  rotation;

    // initialize object names to zero (OpenGL reserved value)
    Transformation() : x(0), y(0), scale(1.0), rotation(0)
    {}
};

// Globals for input
Transformation transformation;
glm::mat3 colour_effects = glm::mat3();
GLfloat gaussian_sigma = 1.5;
bool reset_image = false;
bool pressed = false;

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
    GLuint  vertexBuffer;
    GLuint  colourBuffer;
    GLuint  textureCoordBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), colourBuffer(0), textureCoordBuffer(0), vertexArray(0), elementCount(0)
    {}
};

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry, MyTexture *texture)
{
	GLfloat x_max = 0;
	GLfloat y_max = 0;
	GLfloat x_min = 0;
	GLfloat y_min = 0;

	if(texture->width > texture->height)
	{
		y_max = (texture->height/float(texture->width));
		y_min = -(texture->height/float(texture->width));
		x_max = 1.0;
		x_min = -1.0;
	}
	else if(texture->width < texture->height)
	{
		x_max = (texture->width/float(texture->height));
		x_min = -(texture->width/float(texture->height));
		y_max = 1.0;
		y_min = -1.0;
	}
	else
	{
		y_max = 1.0;
		y_min = -1.0;
		x_max = 1.0;
		x_min = -1.0;
	}

    // associated vertices based on image aspect ratio
    const GLfloat vertices[][2] = {
        { x_min, y_min },
        { x_max, y_min },
        { x_min, y_max },
        { x_min, y_max },
        { x_max, y_max },
        { x_max, y_min }
    };
    const GLfloat colours[][3] = {
        { 1.0, 0.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 },
        { 1.0, 0.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 }
    };

    const GLuint textureCoords[][2] = {
    		{0, texture->height},
			{texture->width, texture->height},
			{0, 0},
			{0, 0},
			{texture->width, 0},
    		{texture->width, texture->height}
    };

    geometry->elementCount = 6;

    // these vertex attribute indices correspond to those specified for the
    // input variables in the vertex shader
    const GLuint VERTEX_INDEX = 0;
    const GLuint COLOUR_INDEX = 1;
    const GLuint TEXTURE_INDEX = 2;

    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // create another one for storing our colours
    glGenBuffers(1, &geometry->colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);

    // Buffer for storing texture
    glGenBuffers(1, &geometry->textureCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);


    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);

    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_INDEX);

    // assocaite the colour array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(COLOUR_INDEX);

    // Set up vertex attribute info for textures
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
    glVertexAttribPointer(TEXTURE_INDEX, 2, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(TEXTURE_INDEX);

    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
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

void RenderScene(MyGeometry *geometry, MyTexture* texture, MyShader *shader)
{
    // clear screen to a dark grey colour
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->program);
    glBindVertexArray(geometry->vertexArray);
    glBindTexture(GL_TEXTURE_RECTANGLE, texture->textureName);
    glUniform4f(glGetUniformLocation(shader->program, "transformation_data"), transformation.x, transformation.y, transformation.scale, transformation.rotation);
    glUniformMatrix3fv(glGetUniformLocation(shader->program, "colour_data"), 1, GL_FALSE, &colour_effects[0][0]);
    glUniform1iv(glGetUniformLocation(shader->program, "kernel"), 9, kernels[filter_type]);
    glUniform1ui(glGetUniformLocation(shader->program, "filter_type"), filter_type);
    glUniform1ui(glGetUniformLocation(shader->program, "gaussian_points"), gaussian_type);
    glUniform1f(glGetUniformLocation(shader->program, "sigma"), gaussian_sigma);
    glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

    // reset state to default (no shader or geometry bound)
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
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
		case GLFW_KEY_1:
			image_type = MANDRILL;
			break;
		case GLFW_KEY_2:
			image_type = UCLOGO;
			break;
		case GLFW_KEY_3:
			image_type = AERIAL;
			break;
		case GLFW_KEY_4:
			image_type = THIRSK;
			break;
		case GLFW_KEY_5:
			image_type = PATTERN;
			break;
		case GLFW_KEY_6:
			image_type = CAPSTONE;
			break;
		case GLFW_KEY_LEFT:
			transformation.rotation += M_PI/12.0;
			break;
		case GLFW_KEY_RIGHT:
			transformation.rotation -= M_PI/12.0;
			break;
		case GLFW_KEY_UP:
			transformation.scale += 0.05;
			break;
		case GLFW_KEY_DOWN:
			transformation.scale -= 0.05;
			break;
		case GLFW_KEY_F1:
		    colour_effects = glm::mat3(1.0);
			break;
		case GLFW_KEY_F2:
			for(GLuint i = 0; i < 3; i++) { colour_effects[i] = glm::vec3(0.333, 0.333, 0.333); }
			break;
		case GLFW_KEY_F3:
			for(GLuint i = 0; i < 3; i++) { colour_effects[i] = glm::vec3(0.299, 0.587, 0.114); }
			break;
		case GLFW_KEY_F4:
			for(GLuint i = 0; i < 3; i++) { colour_effects[i] = glm::vec3(0.222, 0.715, 0.072); }
			break;
        case GLFW_KEY_F5:
			colour_effects[0] = glm::vec3(0.393, 0.769, 0.189);
			colour_effects[1] = glm::vec3(0.349, 0.686, 0.168);
			colour_effects[2] = glm::vec3(0.272, 0.534, 0.131);
		    break;
        case GLFW_KEY_F:
            if(filter_type < GAUSSIAN - 1)
        		filter_type = (FilterType)((int)filter_type + 1);
        	else
        	    filter_type = NO_FILTER;
		    break;
        case GLFW_KEY_G:
        	filter_type = GAUSSIAN;
        	if(gaussian_type == NO_GAUSSIAN)
        	{
        	    gaussian_type = GAUSSIAN_3X3;
        	    gaussian_sigma = 2.0;
        	}
        	else
        	{
        		if(gaussian_type < GAUSSIAN_7X7)
        		{
        		    gaussian_type = (GaussianType)((int)gaussian_type + 2);
        		    gaussian_sigma += 0.22;
        		}
        		else
        		{
        			filter_type = NO_FILTER;
        		    gaussian_type = NO_GAUSSIAN;
        		}
        	}
        	break;
        case GLFW_KEY_SPACE:
        	reset_image = true;
        	colour_effects = glm::mat3(1.0);
        	transformation.rotation = 0;
        	transformation.scale = 1.0;
        	filter_type = NO_FILTER;
        	gaussian_type = NO_GAUSSIAN;

		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_1)
	{
		if(action == GLFW_PRESS)
		{
			pressed = true;
		}
		else if(action == GLFW_RELEASE)
		{
			pressed = false;
		}
	}
}

void mouseMoveCallback(GLFWwindow* window, double x, double y)
{
	if(pressed)
	{
	    transformation.x = ((x)/float(WINDOW_WIDTH)*2) - 1;
	    transformation.y = 1 - ((y)/float(WINDOW_HEIGHT)*2);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset == 1)
		transformation.scale += 0.05;
	else if (yoffset == -1 && transformation.scale >= 0.0)
		transformation.scale -= 0.05;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CPSC 453 OpenGL Assignment 2", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    // set mouse call back functions

    // query and print out information about our OpenGL environment
    QueryGLVersion();

    // call function to load and compile shader programs
    MyShader shader;
    if (!InitializeShaders(&shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }

    // load and initialize the texture
    MyTexture textures[IMAGETYPE_MAX];

    if(!InitializeTexture(&textures[MANDRILL], "image1-mandrill.png"))
        cout << "Failed to load texture!" << endl;

    if(!InitializeTexture(&textures[UCLOGO], "image2-uclogo.png"))
        cout << "Failed to load texture!" << endl;

    if(!InitializeTexture(&textures[AERIAL], "image3-aerial.jpg"))
        cout << "Failed to load texture!" << endl;

    if(!InitializeTexture(&textures[THIRSK], "image4-thirsk.jpg"))
        cout << "Failed to load texture!" << endl;

    if(!InitializeTexture(&textures[PATTERN], "image5-pattern.png"))
        cout << "Failed to load texture!" << endl;
	
    if(!InitializeTexture(&textures[CAPSTONE], "image6-capstone.jpeg"))
        cout << "Failed to load texture!" << endl;

    // call function to create and fill buffers with geometry data
    MyGeometry geometries[IMAGETYPE_MAX];
    if (!InitializeGeometry(&geometries[MANDRILL], &textures[MANDRILL]))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeGeometry(&geometries[UCLOGO], &textures[UCLOGO]))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeGeometry(&geometries[AERIAL], &textures[AERIAL]))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeGeometry(&geometries[THIRSK], &textures[THIRSK]))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeGeometry(&geometries[PATTERN], &textures[PATTERN]))
        cout << "Program failed to intialize geometry!" << endl;
	
    if (!InitializeGeometry(&geometries[CAPSTONE], &textures[CAPSTONE]))
        cout << "Program failed to intialize geometry!" << endl;

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
    	if(reset_image) // reset current image
    	{
    	    if (!InitializeGeometry(&geometries[image_type], &textures[image_type]))
    	        cout << "Program failed to reintialize geometry!" << endl;
    	    reset_image = false;
    	}
        // call function to draw our scene
        RenderScene(&geometries[image_type], &textures[image_type], &shader);

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwWaitEvents();
    }

    // clean up allocated resources before exit
    for(GLuint i = 0; i < IMAGETYPE_MAX; i++)
    {
        DestroyGeometry(&geometries[i]);
    }
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
