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
#include <limits>

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
	FONT_CUSTOM,
	FONT_MAX
};

// --------------------------------------------------------------------------
// Enum used for switching scroll font type

enum ScrollScene
{
	SCROLL_ALEX_BRUSH,
	SCROLL_INCONSOLATA,
	SCROLL_CUSTOM,
	SCROLL_MAX
};

// --------------------------------------------------------------------------
// Globals

#define NAME_SIZE 4
#define SENTENCE_SIZE 27

Scene scene = BEZIER;
BezierScene bezier_scene = BEZIER_QUADRATIC;
FontScene font_scene = FONT_LORA;
ScrollScene scroll_scene = SCROLL_ALEX_BRUSH;

bool show_control_points = true;
bool lines_toggle = false;

GLfloat lastTime = 0;
GLfloat scroll_pos = 0.0;
GLfloat scroll_speed =  0.80;
GLfloat max_x[SCROLL_MAX];

vector<GLfloat> vertices;
vector<GLfloat> control_vertices;
vector<GLfloat> colours;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

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
    string vertexSource = LoadSource("vertex.glsl");
    string fragmentSource = LoadSource("fragment.glsl");
    string tessEvalSource = LoadSource("tess_eval.glsl");
    string tessControlSource = LoadSource("tess_control.glsl");

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
    GLuint  controlVertexBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;
    GLsizei controlElementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), colourBuffer(0), controlVertexBuffer(0), vertexArray(0), elementCount(0), controlElementCount(0)
    {}
};

// --------------------------------------------------------------------------
// Universal geometry functions

GLfloat GetMaxY(vector<GLfloat> *points)
{
	vector<GLfloat> tmp_vec_y;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(!((i + 1) % 2))
			tmp_vec_y.push_back(points->at(i));
	}

	vector<GLfloat>::const_iterator it_max_y;
	it_max_y = max_element(tmp_vec_y.begin(), tmp_vec_y.end());
	return *it_max_y;
}

GLfloat GetMinY(vector<GLfloat> *points)
{
	vector<GLfloat> tmp_vec_y;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(!((i + 1) % 2))
			tmp_vec_y.push_back(points->at(i));
	}

	vector<GLfloat>::const_iterator it_min_y;
	it_min_y = min_element(tmp_vec_y.begin(), tmp_vec_y.end());
	return *it_min_y;
}

GLfloat GetMaxX(vector<GLfloat> *points)
{
	vector<GLfloat> tmp_vec_x;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(((i + 1) % 2))
			tmp_vec_x.push_back(points->at(i));
	}

	vector<GLfloat>::const_iterator it_max_x;
	it_max_x = max_element(tmp_vec_x.begin(), tmp_vec_x.end());
	return *it_max_x;
}

GLfloat GetMinX(vector<GLfloat> *points)
{
	vector<GLfloat> tmp_vec_x;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(((i + 1) % 2))
			tmp_vec_x.push_back(points->at(i));
	}

	vector<GLfloat>::const_iterator it_min_x;
	it_min_x = min_element(tmp_vec_x.begin(), tmp_vec_x.end());
	return *it_min_x;
}

void ScaleVertices(vector<GLfloat> *points, GLfloat scale)
{
	if(scale != 0)
	    for(GLuint i = 0; i < points->size(); i++)
	    {
	    	points->at(i) *= scale;
	    }
}

void CenterYVertices(vector<GLfloat> *points)
{
	vector<GLfloat> tmp_vec_y;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(!((i + 1) % 2))
			tmp_vec_y.push_back(points->at(i));
	}

	vector<GLfloat>::const_iterator it_max_y, it_min_y;
	it_max_y = max_element(tmp_vec_y.begin(), tmp_vec_y.end());
	it_min_y = min_element(tmp_vec_y.begin(), tmp_vec_y.end());
	GLfloat center_y = ((*it_max_y - *it_min_y) / 2) + *it_min_y;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(!((i + 1) % 2))
		{
			if(center_y != 0)
				points->at(i) -= center_y;
		}
	}
}

void CenterVertices(vector<GLfloat> *points)
{
	GLfloat min_x, max_x, min_y, max_y;
	max_x = GetMaxX(points);
	min_x = GetMinX(points);
	max_y = GetMaxY(points);
	min_y = GetMinY(points);
	GLfloat center_x = ((max_x - min_x) / 2) + min_x;
	GLfloat center_y = ((max_y - min_y) / 2) + min_y;
	for(GLuint i = 0; i < points->size(); i++)
	{
		if(((i + 1) % 2))
		{
			if(center_x != 0)
				points->at(i) -= center_x;
		}
		else
		{
			if(center_y != 0)
				points->at(i) -= center_y;
		}
	}

}

void NormalizeVertices(vector<GLfloat> *points)
{
	GLfloat min_x, max_x, min_y, max_y;
	max_x = GetMaxX(points);
	min_x = GetMinX(points);
	max_y = GetMaxY(points);
	min_y = GetMinY(points);

	GLfloat scale_x = fabs(max_x) > fabs(min_x) ? fabs(max_x) : fabs(min_x);
	GLfloat scale_y = fabs(max_y) > fabs(min_y) ? fabs(max_y) : fabs(min_y);
	GLfloat scale = scale_x > scale_y ? scale_x : scale_y;

	if(scale != 0)
	    for(GLuint i = 0; i < points->size(); i++){points->at(i) /= scale;}

}

void AddControlPoint(GLfloat x, GLfloat y, vector<GLfloat> *vertices_vec)
{
	vertices_vec->push_back(x);
	vertices_vec->push_back(y);
}

void AddColour(GLfloat r, GLfloat g, GLfloat b, vector<GLfloat> *colours_vec)
{
	colours_vec->push_back(r);
	colours_vec->push_back(g);
	colours_vec->push_back(b);
}

// --------------------------------------------------------------------------
// Bezier functions

void CubicBezierVertices(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4)
{
	AddControlPoint(x1, y1, &vertices);
	AddColour(1.0, 0.0, 0.0, &colours);
	AddControlPoint(x2, y2, &vertices);
	AddColour(0.0, 0.0, 1.0, &colours);
	AddControlPoint(x3, y3, &vertices);
	AddColour(0.0, 0.0, 1.0, &colours);
	AddControlPoint(x4, y4, &vertices);
	AddColour(1.0, 0.0, 0.0, &colours);

	AddControlPoint(x1, y1, &control_vertices);
	AddControlPoint(x2, y2, &control_vertices);
	AddControlPoint(x2, y2, &control_vertices);
	AddControlPoint(x3, y3, &control_vertices);
	AddControlPoint(x3, y3, &control_vertices);
	AddControlPoint(x4, y4, &control_vertices);
}

void QuadraticBezierVertices(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3)
{
	CubicBezierVertices(x1, y1, x2, y2, x3, y3, x3, y3);
}

void LineBezierVertices(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	CubicBezierVertices(x1, y1, x1, y1, x2, y2, x2, y2);
}

void PointBezierVertices(GLfloat x1, GLfloat y1)
{
	CubicBezierVertices(x1, y1, x1, y1, x1, y1, x1, y1);
}

bool BindGeometryBuffers(MyGeometry *geometry, vector<GLfloat> *vertex_vec, vector<GLfloat> *colour_vec, vector<GLfloat> *control_vec)
{
	GLuint VERTEX_INDEX = 0;
	GLuint COLOUR_INDEX = 1;
	GLuint CONTROL_INDEX = 2;

	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_vec->size()*sizeof(GLfloat), vertex_vec->data(), GL_STATIC_DRAW);

	// create another one for storing our colours
	glGenBuffers(1, &geometry->colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colour_vec->size()*sizeof(GLfloat), colour_vec->data(), GL_STATIC_DRAW);

	glGenBuffers(1, &geometry->controlVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->controlVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, control_vec->size()*sizeof(GLfloat), control_vec->data(), GL_STATIC_DRAW);

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

	// associate the control array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->controlVertexBuffer);
	glVertexAttribPointer(CONTROL_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(CONTROL_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	vertex_vec->clear();
	colour_vec->clear();
	control_vec->clear();

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
    CenterVertices(&vertices);
    NormalizeVertices(&vertices);

    CenterVertices(&control_vertices);
    NormalizeVertices(&control_vertices);

    geometry->elementCount = vertices.size()/2;
    geometry->controlElementCount = control_vertices.size()/2;
	return BindGeometryBuffers(geometry, &vertices, &colours, &control_vertices);
}

bool InitializeQuadraticBezier(MyGeometry *geometry)
{
	QuadraticBezierVertices(1.0, 1.0, 2.0, -1.0, 0, -1.0);
	QuadraticBezierVertices(0, -1, -2, -1, -1, 1);
	QuadraticBezierVertices(-1, 1, 0, 1, 1, 1);
	QuadraticBezierVertices(1.2, 0.5, 2.5, 1.0, 1.2, -0.4);
    CenterVertices(&vertices);
    NormalizeVertices(&vertices);

    CenterVertices(&control_vertices);
    NormalizeVertices(&control_vertices);

    geometry->elementCount = vertices.size()/2;
    geometry->controlElementCount = control_vertices.size()/2;
	return BindGeometryBuffers(geometry, &vertices, &colours, &control_vertices);
}

enum SegmentDegree
{
	POINT,
	LINE,
	QUAD,
	CUBIC
};

bool InitializeWord(MyGeometry *geometry, MyGlyph *name, GLuint word_size)
{
	GLfloat x_alignment = 0;
    for(GLuint i = 0; i < word_size; i++)
    {   GLfloat max_x = -std::numeric_limits<float>::max();
    	for(GLuint j = 0; j < name[i].contours.size(); j++)
    	{
    		for(GLuint k = 0; k < name[i].contours[j].size(); k++)
    		{
    			GLuint degree = name[i].contours[j][k].degree;
    			MySegment segment = name[i].contours[j][k];
    			switch(degree)
    			{
    			case POINT:
                    PointBezierVertices(segment.x[0] + x_alignment, segment.y[0]);
    				break;
    			case LINE:
    				LineBezierVertices(segment.x[0] + x_alignment, segment.y[0], segment.x[1] + x_alignment, segment.y[1]);
    				break;
    			case QUAD:
    				QuadraticBezierVertices(segment.x[0] + x_alignment, segment.y[0], segment.x[1] + x_alignment, segment.y[1], segment.x[2] + x_alignment, segment.y[2]);
    				break;
    			case CUBIC:
    				CubicBezierVertices(segment.x[0] + x_alignment, segment.y[0], segment.x[1] + x_alignment, segment.y[1], segment.x[2] + x_alignment, segment.y[2], segment.x[3] + x_alignment, segment.y[3]);
    				break;
    			}

    			for(GLuint l = 0; l < segment.degree; l++)
    			{
    				if (segment.x[l] > max_x)
    				{
    					max_x = segment.x[l];
    				}
    			}
    		}
    	}
    	x_alignment += (max_x + 0.1);
    }
    CenterVertices(&vertices);
    NormalizeVertices(&vertices);

    CenterVertices(&control_vertices);
    NormalizeVertices(&control_vertices);

    geometry->elementCount = vertices.size()/2;
    geometry->controlElementCount = control_vertices.size()/2;
	return BindGeometryBuffers(geometry, &vertices, &colours, &control_vertices);
}

void InitializeWordAppend(MyGlyph *name, GLuint word_size, GLfloat &x_alignment)
{
    for(GLuint i = 0; i < word_size; i++)
    {   GLfloat max_x = -std::numeric_limits<float>::max();
    	for(GLuint j = 0; j < name[i].contours.size(); j++)
    	{
    		for(GLuint k = 0; k < name[i].contours[j].size(); k++)
    		{
    			GLuint degree = name[i].contours[j][k].degree;
    			MySegment segment = name[i].contours[j][k];
    			switch(degree)
    			{
    			case POINT:
                    PointBezierVertices(segment.x[0] + x_alignment, segment.y[0]);
    				break;
    			case LINE:
    				LineBezierVertices(segment.x[0] + x_alignment, segment.y[0], segment.x[1] + x_alignment, segment.y[1]);
    				break;
    			case QUAD:
    				QuadraticBezierVertices(segment.x[0] + x_alignment, segment.y[0], segment.x[1] + x_alignment, segment.y[1], segment.x[2] + x_alignment, segment.y[2]);
    				break;
    			case CUBIC:
    				CubicBezierVertices(segment.x[0] + x_alignment, segment.y[0], segment.x[1] + x_alignment, segment.y[1], segment.x[2] + x_alignment, segment.y[2], segment.x[3] + x_alignment, segment.y[3]);
    				break;
    			}

    			for(GLuint l = 0; l < segment.degree; l++)
    			{
    				if (segment.x[l] > max_x)
    				{
    					max_x = segment.x[l];
    				}
    			}
    		}
    	}
    	x_alignment += (max_x + 0.05*0.7);
    }
}

bool InitializeSentence(MyGeometry *geometry, GlyphExtractor *ge, char *sentence, ScrollScene font)
{
	char *tmp, *sentence_ptr;
	sentence_ptr = sentence;
	tmp = sentence;
	GLfloat alignment = 0.0;

	while(*tmp != '\0')
	{
	    GLuint word_size = 0;
	    for (; *tmp != '.' && *tmp != ' '; tmp++) {
	    	word_size++;
	    }

	    char word[word_size + 1];
	    memcpy(word, sentence_ptr, word_size);
	    word[word_size] = '\0';
	    sentence_ptr += word_size + 1;

        MyGlyph glyph_word[word_size];
        for(GLuint i = 0; i < word_size; i++) { glyph_word[i] =  ge->ExtractGlyph(word[i]); }

        InitializeWordAppend(glyph_word, word_size, alignment);
        alignment += 0.25;

        tmp++;
	}

    CenterYVertices(&vertices);
    ScaleVertices(&vertices, 0.7);
    max_x[font] = GetMaxX(&vertices);
    geometry->elementCount = vertices.size()/2;
    geometry->controlElementCount = control_vertices.size()/2;
	return BindGeometryBuffers(geometry, &vertices, &colours, &control_vertices);
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
// Rendering functions that draw our scene to the frame buffer

void GetScrollRate()
{
    GLfloat currentTime;
	// Measure speed
    currentTime = glfwGetTime();

    scroll_pos += scroll_speed/(1/GLfloat(currentTime - lastTime));

   if((max_x[scroll_scene] - scroll_pos) < -1.0)
   {
       scroll_pos = -1.0;
   }

}

void RenderBezierCurves(MyGeometry *geometry, MyShader *shader)
{
    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->tessProgram);
    if(scene == BEZIER)
        glUniform1ui(glGetUniformLocation(shader->tessProgram, "bezier_scene"), bezier_scene);
    else
    	glUniform1ui(glGetUniformLocation(shader->tessProgram, "bezier_scene"), BEZIER_CUBIC);
    glUniform1i(glGetUniformLocation(shader->tessProgram, "control_points"), false);
    glUniform1i(glGetUniformLocation(shader->tessProgram, "control_lines"), false);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glBindVertexArray(geometry->vertexArray);
    glUniform1f(glGetUniformLocation(shader->tessProgram, "scroll_pos"), scroll_pos);
    glDrawArrays(GL_PATCHES, 0, geometry->elementCount);

    // reset state to default (no shader or geometry bound)
    glBindVertexArray(0);
    glUseProgram(0);
}

void RenderControlPoints(MyGeometry *geometry, MyShader *shader)
{
    // Draw control points and polygons for bezier curves
    if(show_control_points && scene != SCROLLING_TEXT)
    {
        glBindVertexArray(geometry->vertexArray);
        glUseProgram(shader->controlProgram);
        glUniform1i(glGetUniformLocation(shader->controlProgram, "control_lines"), true);
        glUniform1i(glGetUniformLocation(shader->controlProgram, "control_points"), false);
        glDrawArrays(GL_LINES, 0, geometry->controlElementCount);

        glUniform1i(glGetUniformLocation(shader->controlProgram, "control_lines"), false);
        glUniform1i(glGetUniformLocation(shader->controlProgram, "control_points"), true);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glDrawArrays(GL_POINTS, 0, geometry->elementCount);


        glDisable(GL_PROGRAM_POINT_SIZE);

        // reset state to default (no shader or geometry bound)
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void RenderScene(MyGeometry *geometry, MyShader *shader)
{
    // clear screen to a dark grey colour
    //glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderBezierCurves(geometry, shader);
    RenderControlPoints(geometry, shader);

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(scene == SCROLLING_TEXT)
    {
	    if(yoffset == 1)
	    	scroll_speed += 0.10;
	    else if (yoffset == -1 && scroll_speed >= 0.0)
	    	scroll_speed -= 0.10;
    }
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
			scroll_pos = 0.0;
			scene = BEZIER;
			break;
		case GLFW_KEY_2:
			scroll_pos = 0.0;
			scene = FONTS;
			break;
		case GLFW_KEY_3:
			scroll_pos = -1.0;
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
		case GLFW_KEY_C:
				show_control_points = !show_control_points;
				break;
		case GLFW_KEY_W:
			if(scene == SCROLLING_TEXT)
				scroll_speed += 0.10;
			    break;
		case GLFW_KEY_S:
			if(scene == SCROLLING_TEXT && scroll_speed >= 0.00)
				scroll_speed -= 0.10;
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
    glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(512, 512, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    char name[5] = "Matt";

    //Load lora font and print some info
  	GlyphExtractor* ge = new GlyphExtractor();
    if(ge->LoadFontFile("Lora-Bold.ttf"))
      cout << "font Lora-Bold.ttf loaded" << endl;
    MyGlyph lora_name[NAME_SIZE];
    for(GLuint i = 0; i < NAME_SIZE; i++) { lora_name[i] =  ge->ExtractGlyph(name[i]); }

    if(ge->LoadFontFile("SourceSansPro-Bold.otf"))
      cout << "font SourceSansPro-Bold.otf loaded" << endl;
    MyGlyph ssp_name[NAME_SIZE];
    for(GLuint i = 0; i < NAME_SIZE; i++) { ssp_name[i] =  ge->ExtractGlyph(name[i]); }

    if(ge->LoadFontFile("AlexBrush-Regular.ttf"))
      cout << "font AlexBrush-Regular.ttf loaded" << endl;
    MyGlyph alex_brush_name[NAME_SIZE];
    for(GLuint i = 0; i < NAME_SIZE; i++) { alex_brush_name[i] =  ge->ExtractGlyph(name[i]); }

    char sentence[46] = "The quick brown fox jumps over the lazy dog.";

    // query and print out information about our OpenGL environment
    QueryGLVersion();

    // call function to create and fill buffers with geometry data for bezier scenes
    MyGeometry bezier_geometry[BEZIER_MAX];
    if (!InitializeQuadraticBezier(&bezier_geometry[BEZIER_QUADRATIC]))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeCubicBezier(&bezier_geometry[BEZIER_CUBIC]))
        cout << "Program failed to intialize geometry!" << endl;


    // call function to create and fill buffers with geometry data for font scene
    MyGeometry name_geometry[FONT_MAX];
    if (!InitializeWord(&name_geometry[FONT_LORA], lora_name, NAME_SIZE))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeWord(&name_geometry[FONT_SOURCE_SANS_PRO], ssp_name, NAME_SIZE))
        cout << "Program failed to intialize geometry!" << endl;

    if (!InitializeWord(&name_geometry[FONT_CUSTOM], alex_brush_name, NAME_SIZE))
        cout << "Program failed to intialize geometry!" << endl;


    // call function to create and fill buffers with geometry data for font scene
    MyGeometry scroll_geometry[SCROLL_MAX];

    //Load sentence for scrolling text
    if(ge->LoadFontFile("AlexBrush-Regular.ttf"))
      cout << "font AlexBrush-Regular.ttf loaded" << endl;

    if (!InitializeSentence(&scroll_geometry[SCROLL_ALEX_BRUSH], ge, sentence, SCROLL_ALEX_BRUSH))
        cout << "Program failed to intialize geometry!" << endl;

    if(ge->LoadFontFile("Inconsolata.otf"))
      cout << "font Inconsolata.otf loaded" << endl;

    if (!InitializeSentence(&scroll_geometry[SCROLL_INCONSOLATA], ge, sentence, SCROLL_INCONSOLATA))
        cout << "Program failed to intialize geometry!" << endl;

    if(ge->LoadFontFile("Lora-Bold.ttf"))
      cout << "font Lora-Bold.ttf loaded" << endl;

    if (!InitializeSentence(&scroll_geometry[SCROLL_CUSTOM], ge, sentence, SCROLL_CUSTOM))
        cout << "Program failed to intialize geometry!" << endl;

    // call function to load and compile shader programs
    MyShader shader;
    if (!InitializeShaders(&shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }

    MyGeometry *geometry_select;

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        // call function to draw our scene
    	switch(scene)
    	{
        case BEZIER:
        	geometry_select = &bezier_geometry[bezier_scene];
        	break;
        case FONTS:
        	geometry_select = &name_geometry[font_scene];
        	break;
        case SCROLLING_TEXT:
        	// Get FPS to properly calculate scroll rate
        	lastTime = glfwGetTime();
        	geometry_select = &scroll_geometry[scroll_scene];
        	break;
    	}
        RenderScene(geometry_select, &shader);

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();
        if (scene == SCROLLING_TEXT)
          GetScrollRate();
    }

	// clean up allocated resources before exit
    for(GLuint i = 0; i < sizeof(name_geometry)/sizeof(name_geometry[0]); i++){DestroyGeometry(&name_geometry[i]);}
	for(GLuint i = 0; i < sizeof(bezier_geometry)/sizeof(bezier_geometry[0]); i++){DestroyGeometry(&bezier_geometry[i]);}
	for(GLuint i = 0; i < sizeof(scroll_geometry)/sizeof(scroll_geometry[0]); i++){DestroyGeometry(&scroll_geometry[i]);}
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
