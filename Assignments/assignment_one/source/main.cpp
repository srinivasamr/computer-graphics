// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

#include <common/shader.hpp>
#include <common/controls.hpp>

char g_file_names[3][100] = {
	"data/w.txt",
	"data/a.txt",
	"data/b.txt"
};

#define DEBUG
#ifdef DEBUG
#define ASGN_LOG printf
#else
#define ASGN_LOG 
#endif

int g_num_curves;
int g_num_points;
int g_num_points_to_plot;

class Point
{
public:
	Point(){}
	Point(float x, float y){}
	~Point(){}
	float _x;
	float _y;
};

class Curve
{
public:
	Curve(){}
	~Curve(){}
	int _no_points;
	int _no_control_points;
	Point *_pPoints;
};


Point *g_pPoints;
Curve *g_pCurve;
int *pC;

void 
ReadControlPoints(char *file_name) 
{
	float x;
	float y;
	int no_points;
	int no_plot;
	Point *pPoints;
	ASGN_LOG("Enter");
	ifstream cpStream(file_name);
	cpStream >> g_num_curves;
	cout << "number of cruves:"<< g_num_curves << endl;
	g_pCurve = new Curve[g_num_curves];

	for (int i = 0; i < g_num_curves; i++)
	{
		pPoints = NULL;
		cpStream >> no_points;
		cpStream >> no_plot;
		g_pCurve[i]._no_points = no_points;
		g_pCurve[i]._no_control_points = no_plot;
		pPoints = new Point[no_points];
		for (int k = 0; k < no_points; k++)
		{
			cpStream >> x;
			cpStream >> y;
			pPoints[k]._x = x;
			pPoints[k]._y = y;
			ASGN_LOG("reading: %f, %f\n", x, y);
		}
		g_pCurve[i]._pPoints = pPoints;
	}
	cpStream.close();
	ASGN_LOG("Exit\n");
}

void bezierCoefficients(int n,int *c)
{
	int k,i;
	for(k=0;k<=n;k++)
	{
		c[k]=1;
		for(i=n;i>=k+1;i--) {
			c[k]*=i;
		}
		for(i=n-k;i>=2;i--) {
			c[k]/=i;
		}

	}
}

void
GetBeizerCurve(int index, GLfloat **ppVertexBufferData, int &num_vertex, int &num_curves)
{
	int *pBezierPoints;
	int n;
	int vertex_count;
	float x,y,u,blend;
	float max_x = 0.0;
	float max_y = 0.0;
	index = 0;
	GLfloat *pVertexBufferData;
	GLfloat *pTemp;
	ReadControlPoints(g_file_names[0]);
	pVertexBufferData = new GLfloat[102 * 3 * g_num_curves];
	num_curves = g_num_curves;
	num_vertex = 0;

	for (int c = 0 ; c < num_curves; c++)
	{
		int no_control_points;
		int no_points;
		Curve *pCurve;
		Point *pPoints;

		pTemp = pVertexBufferData + 100*3*c;

		index = 0;
		pCurve = &g_pCurve[c];
		pPoints = pCurve->_pPoints;
		no_control_points = pCurve->_no_control_points;
		no_points = pCurve->_no_points;

		pBezierPoints = new int[no_control_points];
		n = no_control_points - 1;
		bezierCoefficients(n, pBezierPoints);

		ASGN_LOG("Curve no - %d\n", c);
		vertex_count = 0;
		for(u = 0; u < 1; u = u + .01)
		{
			x = 0; y = 0;
			index = 0;
			
			for (int k = 0; k < no_control_points; k++)
			{
				blend = pBezierPoints[k] * pow(u,k) * pow(1-u,n-k);
				index = index + no_points/no_control_points;
				if (index < no_points)
				{
					x += pPoints[index]._x*blend;
					y += pPoints[index]._y*blend;
				}
			}
			pTemp[vertex_count * 3] = x;
			pTemp[vertex_count * 3 + 1] = y;
			pTemp[vertex_count * 3 + 2] = 0.0;
			if (x > max_x) {
				max_x = x;
			}
			if (y > max_y)
			{
				max_y = y;
			}
			vertex_count++;
		}
		delete []pBezierPoints;
		pBezierPoints = NULL;
		pTemp[297] = pTemp[0];
		pTemp[298] = pTemp[1];
	}
	for (int z = 0; z < (100 * num_curves); z++)
	{
		pVertexBufferData[z*3] = pVertexBufferData[z*3]/max_x;
		pVertexBufferData[z*3 + 1] = pVertexBufferData[z*3 + 1]/max_y;
	}
	*ppVertexBufferData = pVertexBufferData;
	num_vertex = 100;
}


int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 0 - Keyboard and Mouse", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	GLfloat *pVertexData = NULL;

	int num_vertex;
	int num_curves;
	//Form the vertices by making beizer curve
	GetBeizerCurve(0, &pVertexData, num_vertex, num_curves);
	ASGN_LOG("No of vertex = %d\n", num_vertex, num_curves);
	

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_vertex * sizeof(GLfloat) * 3 * num_curves, pVertexData, GL_STATIC_DRAW);

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the line segment !
		for (int cur = 0; cur < num_curves; cur++) {
			glDrawArrays(GL_LINE_STRIP, cur*num_vertex, num_vertex); // each curve has 100 vertex
		}
		

		delete []pVertexData;
		pVertexData = NULL;

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

