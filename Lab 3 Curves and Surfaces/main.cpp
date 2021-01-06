#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iterator>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <GLFW/glfw3.h>

#include "utils/loadTexture.h"
#include "utils/meshio.h"
#include "utils/mathTool.h"

#pragma comment( lib, "OpenGL32.lib" )
using std::string;
using glm::quat;
using glm::vec3;
using glm::mat4;
using glm::mat3;

int wwidth = 1024, wheight = 768;
GLFWwindow* window;

// object view in program 
GLuint program0;
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint vertexbuffer;
GLuint normalbuffer;

// curve view in program 
GLuint program1;
GLuint MVPID;
GLuint ColorID;
GLuint curvevertexbuffer;

GLuint vao;

const float viewMoveSpeed = 0.04f;
const float editMoveSpeed = 0.04f;
const float viewRotateSpeed = 0.01f;
const float editRotateSpeed = 0.025f;

float vHorizontalAngle = -glm::pi<float>();
float vVerticalAngle = 0;
glm::vec3 vPosition = glm::vec3(0, 0, 5);
glm::vec3 vDirection;
glm::vec3 vRight;
glm::vec3 vUp;

// view matrix
const mat4 ProjectionMatrix = glm::perspective(
	glm::radians(60.0f), (float)wwidth / wheight, 1.0f, 100.0f);
glm::mat4 MVP;
glm::mat4 ViewMatrix;
quat ModelSORRo = quat(1, 0, 0, 0);
glm::mat4 ModelMatrixSORTr = glm::mat4(1.0);
quat ModelSSRo = quat(1, 0, 0, 0);
glm::mat4 ModelMatrixSSTr = glm::mat4(1.0);
glm::mat4 ModelTransMatrix = glm::mat4(1.0);
quat ModelRoQuat = quat(1, 0, 0, 0);
mat4 ModelMatrix = glm::mat4(1.0);

// user control
bool viewMode = true;
bool PointCurveMode = true;
bool show2 = true;
bool show4 = false;
bool show5 = false;
// surface of revolution
bool SORMode = false;
// sweep surface
bool SSMode = false;

vector<float> SORProfile = {
	0.000000 ,-0.459543,
	-0.211882, -0.456747,
	-0.421882, -0.436747,
	-0.848656, -0.278898,

	-1.112097, 0,
	-1.2, 0.384005,
	-1.164785, 1.105511,

	-1.104785, 1.5,
	-1.044785, 1.9,
	-0.991667, 2.328629,

	-0.98, 2.503360,
	-1.08, 2.503360,
	-1.088800, 2.345600,

	-1.15, 1.9,
	-1.22, 1.5,
	-1.278000, 1.162800,

	-1.324800, 0.085200,
	-1.154800, -0.225200,
	-0.915600 ,-0.381200,

	-0.380400 ,-0.622000,
	-0.380400 ,-0.622000,
	-0.144000, -0.968400,

	-0.086800, -1.380000,
	-0.086800, -1.480000,
	-0.128400, -2.112400,

	-0.257200, -2.212800,
	-0.407200, -2.232800,
	-0.994400, -2.262800,

	-1.214800, -2.303200,
	-1.199200, -2.428400,
	-1.057600, -2.458800,

	0.000000, -2.458802
};

vector<float> SSProfile =
{
	0.000000, -0.250000,
	0.625,-0.875,
	0.875,-0.625,
	0.250000 ,0.000000,

	0.875,0.625,
	0.625,0.875,
	0.000000, 0.250000,

	-0.625,0.875,
	-0.875,0.625,
	-0.250000, 0.000000,

	-0.875,-0.625,
	-0.625,-0.875
};

vector<float> SSCurve = {
1.60, -0.255455, -0.080661,
1.582727, 1.374545, -1.050661,
0.912727, 2.034545, -0.760661,
0.242727, 2.094545, -0.410661,

-1.097273, 1.444545, 0.259339,
-1.427273, 0.424545, 0.929339,
-1.5, -0.4, 0.6,

-1.767273, -1.585455, 0.371157,
0.912727, -1.585455, -0.278843,
};

const bool close2 = false;
const bool close4 = false;
const bool close5 = false;

// for vexter order, thus in counterclockwise order from below view point
// there exist many circumstances for determing the viewing direction 
const vec3 zview = vec3(0, 0, 1);
const vec3 yview = vec3(0, 1, 0);
// profile curve 
const int ptInterval = 50;
// surface of revolution
const int angleInterval = 50;
// swept surfaces
const int stInterval = 50;

vector<vec3> curveVertex;
vector<vec3> curveTan;
vector<vec3> curveNormal;
vector<vec3> curveB;
vector<vec3> faceVertex;
vector<vec3> faceNormal;


vector<vector<vec3>> curveVertexs;
vector<vector<vec3>> curveTans;
vector<vector<vec3>> curveNormals;
vector<vector<vec3>> curveBs;
vector<vector<vec3>> faceVertexs;
vector<vector<vec3>> faceNormals;


bool escPress = false;
// transitions
bool wPress = false;
bool aPress = false;
bool sPress = false;
bool dPress = false;
bool qPress = false;
bool ePress = false;
bool tPress = false;
bool fPress = false;
bool gPress = false;
bool hPress = false;
bool rPress = false;
bool yPress = false;

void resetCamera()
{
	vHorizontalAngle = -glm::pi<float>();
	vVerticalAngle = 0;
	vPosition = glm::vec3(0, 0, 5);
}

void resetModel()
{
	if (SORMode)
	{
		ModelTransMatrix = ModelMatrixSORTr;
		ModelRoQuat = ModelSORRo;
	}
	else if (SSMode)
	{
		ModelTransMatrix = ModelMatrixSSTr;
		ModelRoQuat = ModelSSRo;
	}
	else
	{
		ModelTransMatrix = glm::mat4(1.0);
		ModelRoQuat = quat(1, 0, 0, 0);
	}
	ModelMatrix = ModelTransMatrix * glm::toMat4(ModelRoQuat);
}

void init()
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		exit(-1);
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_VISIBLE, false);
	glfwWindowHint(GLFW_RESIZABLE, false);

	window = glfwCreateWindow(wwidth, wheight, "exp03", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}

	printf("press '1' to toggle transformation mode for object\n"
		"press 'wasdqe' to move object or camera depending on corrent mode\n"
		"press 'fghtry' to rotate object or camera depending on corrent mode\n"
		"press '2,4,5' to enter curve dislaying mode\n"
		"press 'z' to toggle control point mode\n"
		"press '3' to show surface of revolution based on what displayed in '2'\n"
		"press '6' to show swept surface based on what displayed in '4' as profile curve and in '5' as sweep curve\n"
		"press 'esc' to ternimate program.\n"
	);
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		if (action == GLFW_PRESS)
		{
			if (key == GLFW_KEY_ESCAPE)
			{
				escPress = true;
			}
			else if (key == GLFW_KEY_1)
			{
				if (viewMode && !show2 && !show4 && !show5)
				{
					resetModel();
					printf("entering transformation mode\n");
					viewMode = !viewMode;
				}
				// edit model mode
				else if (!viewMode && !show2 && !show4 && !show5)
				{
					if (SORMode)
					{
						ModelMatrixSORTr = ModelTransMatrix;
						ModelSORRo = ModelRoQuat;
					}
					else if (SSMode)
					{
						ModelMatrixSSTr = ModelTransMatrix;
						ModelSSRo = ModelRoQuat;
					};
					printf("exiting transformation mode\n");
					viewMode = !viewMode;
				}

			}
			else if (viewMode && key == GLFW_KEY_2)
			{
				PointCurveMode = true;
				SORMode = false;
				SSMode = false;
				show2 = true;
				show4 = false;
				show5 = false;
				resetCamera();
				resetModel();
				printf("displaying profile curve for surfaces of revolution\n");
				printf("entering control point mode\n");
			}
			else if (!SORMode && viewMode && key == GLFW_KEY_3)
			{
				PointCurveMode = false;
				SORMode = true;
				SSMode = false;
				show2 = false;
				show4 = false;
				show5 = false;
				resetCamera();
				resetModel();
				printf("entering surfaces of revolution mode\n");

			}
			else if (viewMode && key == GLFW_KEY_4)
			{
				PointCurveMode = true;
				SORMode = false;
				SSMode = false;
				show2 = false;
				show4 = true;
				show5 = false;
				resetCamera();
				resetModel();
				printf("displaying profile curve for swept surfaces\n");
				printf("entering control point mode\n");
			}
			else if (viewMode && key == GLFW_KEY_5)
			{
				PointCurveMode = true;
				SORMode = false;
				SSMode = false;
				show2 = false;
				show4 = false;
				show5 = true;
				resetCamera();
				resetModel();
				printf("displaying sweep curve for swept surfaces\n");
				printf("entering control point mode\n");
			}
			else if (viewMode && key == GLFW_KEY_6)
			{

				PointCurveMode = false;
				SORMode = false;
				SSMode = true;
				show2 = false;
				show4 = false;
				show5 = false;
				resetCamera();
				resetModel();
				printf("entering swept surfaces mode\n");
			}
			else if ((show2 || show4 || show5) && key == GLFW_KEY_Z)
			{
				if (!PointCurveMode)
				{
					printf("entering control point mode\n");
				}
				else
				{
					printf("exiting control point mode\n");
				}
				PointCurveMode = !PointCurveMode;
			}
			if (key == GLFW_KEY_A)
			{
				aPress = true;
			}
			else if (key == GLFW_KEY_S)
			{
				sPress = true;
			}
			else if (key == GLFW_KEY_D)
			{
				dPress = true;
			}
			else if (key == GLFW_KEY_W)
			{
				wPress = true;
			}
			else if (key == GLFW_KEY_Q)
			{
				qPress = true;
			}
			else if (key == GLFW_KEY_E)
			{
				ePress = true;
			}
			else if (key == GLFW_KEY_F)
			{
				fPress = true;
			}
			else if (key == GLFW_KEY_G)
			{
				gPress = true;
			}
			else if (key == GLFW_KEY_H)
			{
				hPress = true;
			}
			else if (key == GLFW_KEY_T)
			{
				tPress = true;
			}
			else if (key == GLFW_KEY_R)
			{
				rPress = true;
			}
			else if (key == GLFW_KEY_Y)
			{
				yPress = true;
			}
		}
		else if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_A)
			{
				aPress = false;
			}
			else if (key == GLFW_KEY_S)
			{
				sPress = false;
			}
			else if (key == GLFW_KEY_D)
			{
				dPress = false;
			}
			else if (key == GLFW_KEY_W)
			{
				wPress = false;
			}
			else if (key == GLFW_KEY_Q)
			{
				qPress = false;
			}
			else if (key == GLFW_KEY_E)
			{
				ePress = false;
			}
			else if (key == GLFW_KEY_F)
			{
				fPress = false;
			}
			else if (key == GLFW_KEY_G)
			{
				gPress = false;
			}
			else if (key == GLFW_KEY_H)
			{
				hPress = false;
			}
			else if (key == GLFW_KEY_T)
			{
				tPress = false;
			}
			else if (key == GLFW_KEY_R)
			{
				rPress = false;
			}
			else if (key == GLFW_KEY_Y)
			{
				yPress = false;
			}
		}
		});

	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(
		window,
		(vidmode->width - wwidth) / 2,
		(vidmode->height - wheight) / 2
	);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW
	glewExperimental = true; 
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(-1);
	};

	// init shaders
	program0 = LoadShaders("object.vs.glsl", "object.fs.glsl");
	MatrixID = glGetUniformLocation(program0, "MVP");
	ViewMatrixID = glGetUniformLocation(program0, "V");
	ModelMatrixID = glGetUniformLocation(program0, "M");

	program1 = LoadShaders("curve.vs.glsl", "curve.fs.glsl");
	MVPID = glGetUniformLocation(program1, "MVP");
	ColorID = glGetUniformLocation(program1, "lcolor");

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vertexbuffer);
	glGenBuffers(1, &normalbuffer);
	glGenBuffers(1, &curvevertexbuffer);


	glfwShowWindow(window);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
}

void viewModel()
{
	vDirection = glm::vec3(
		cos(vVerticalAngle) * sin(vHorizontalAngle),
		sin(vVerticalAngle),
		cos(vVerticalAngle) * cos(vHorizontalAngle)
	);

	vRight = vec3(
		sin(vHorizontalAngle - glm::pi<float>() / 2.0f),
		0,
		cos(vHorizontalAngle - glm::pi<float>() / 2.0f)
	);

	vUp = glm::cross(vRight, vDirection);

	if (!viewMode)
	{
		vec3 mTranslate = glm::vec3(0, 0, 0);
		float modelDirectionAngle = 0.0f;
		float modelRightAngle = 0.0f;
		float modelUpAngle = 0.0f;

		if (aPress == true)
		{
			mTranslate -= vRight * editMoveSpeed;
		}
		else if (sPress == true)
		{
			mTranslate -= vDirection * editMoveSpeed;
		}
		else if (dPress == true)
		{
			mTranslate += vRight * editMoveSpeed;
		}
		else if (wPress == true)
		{
			mTranslate += vDirection * editMoveSpeed;
		}
		else if (qPress == true)
		{
			mTranslate += vUp * editMoveSpeed;
		}
		else if (ePress == true)
		{
			mTranslate -= vUp * editMoveSpeed;
		}
		else if (fPress == true)
		{
			modelUpAngle -= viewRotateSpeed;
		}
		else if (gPress == true)
		{
			modelRightAngle += viewRotateSpeed;
		}
		else if (hPress == true)
		{
			modelUpAngle += viewRotateSpeed;
		}
		else if (tPress == true)
		{
			modelRightAngle -= viewRotateSpeed;
		}
		else if (rPress == true)
		{
			modelDirectionAngle -= viewRotateSpeed;

		}
		else if (yPress == true)
		{
			modelDirectionAngle += viewRotateSpeed;
		}

		ModelTransMatrix = glm::translate(ModelTransMatrix, mTranslate);

		glm::quat ModelDirectionQuaternion = glm::quat(
			cos(modelDirectionAngle / 2),
			vDirection.x * sin(modelDirectionAngle / 2),
			vDirection.y * sin(modelDirectionAngle / 2),
			vDirection.z * sin(modelDirectionAngle / 2)
		);

		glm::quat ModelUpQuaternion = glm::quat(
			cos(modelUpAngle / 2),
			vUp.x * sin(modelUpAngle / 2),
			vUp.y * sin(modelUpAngle / 2),
			vUp.z * sin(modelUpAngle / 2)
		);

		glm::quat ModelRightQuaternion = glm::quat(
			cos(modelRightAngle / 2),
			vRight.x * sin(modelRightAngle / 2),
			vRight.y * sin(modelRightAngle / 2),
			vRight.z * sin(modelRightAngle / 2)
		);

		ModelRoQuat =
			ModelDirectionQuaternion *
			ModelUpQuaternion *
			ModelRightQuaternion *
			ModelRoQuat;
		modelRightAngle = modelUpAngle = modelDirectionAngle = 0;
		ModelMatrix = ModelTransMatrix * glm::toMat4(ModelRoQuat);
	}
	else
	{
		if (aPress == true)
		{
			vPosition -= vRight * viewMoveSpeed;
		}
		else if (sPress == true)
		{
			vPosition -= vDirection * viewMoveSpeed;
		}
		else if (dPress == true)
		{
			vPosition += vRight * viewMoveSpeed;
		}
		else if (wPress == true)
		{
			vPosition += vDirection * viewMoveSpeed;
		}
		else if (qPress == true)
		{
			vPosition += vUp * viewMoveSpeed;
		}
		else if (ePress == true)
		{
			vPosition -= vUp * viewMoveSpeed;
		}
		else if (fPress == true)
		{
			vHorizontalAngle += viewRotateSpeed;
		}
		else if (gPress == true)
		{
			vVerticalAngle -= viewRotateSpeed;
		}
		else if (hPress == true)
		{
			vHorizontalAngle -= viewRotateSpeed;
		}
		else if (tPress == true)
		{
			vVerticalAngle += viewRotateSpeed;
		}

		vDirection = vec3(
			cos(vVerticalAngle) * sin(vHorizontalAngle),
			sin(vVerticalAngle),
			cos(vVerticalAngle) * cos(vHorizontalAngle)
		);

		vRight = vec3(
			sin(vHorizontalAngle - glm::pi<float>() / 2.0f),
			0,
			cos(vHorizontalAngle - glm::pi<float>() / 2.0f)
		);

		vUp = glm::cross(vRight, vDirection);
	}

	ViewMatrix = glm::lookAt(
		vPosition,
		vPosition + vDirection,
		vUp
	);


	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
}


void loop()
{
	while (escPress != true && glfwWindowShouldClose(window) == 0)
	{
		glViewport(0, 0, wwidth, wheight);
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		viewModel();

		if (SORMode || SSMode)
		{
			if (SORMode)
			{
				faceVertex = faceVertexs[0];
				faceNormal = faceNormals[0];
			}
			else
			{
				faceVertex = faceVertexs[1];
				faceNormal = faceNormals[1];
			}
			
			glUseProgram(program0);

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			glBindVertexArray(vao);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER,
				faceVertex.size() * sizeof(glm::vec3), &faceVertex[0], GL_STATIC_DRAW);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glBufferData(GL_ARRAY_BUFFER,
				faceNormal.size() * sizeof(glm::vec3), &faceNormal[0], GL_STATIC_DRAW);
			glVertexAttribPointer(
				1,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);
			glDrawArrays(GL_TRIANGLES, 0, faceVertex.size());

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glUseProgram(0);
		}
		else if (PointCurveMode)
		{
			if (show2)
			{
				curveVertex = curveVertexs[0];
			}
			else if (show4)
			{
				curveVertex = curveVertexs[2];
			}
			else if (show5)
			{
				curveVertex = curveVertexs[4];
			}
			
			glUseProgram(program1);

			glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
			// blue
			glUniform3f(ColorID, 0.0f, 0.0f, 1.0f);
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, curvevertexbuffer);
			glBufferData(GL_ARRAY_BUFFER,
				curveVertex.size() * sizeof(glm::vec3), &curveVertex[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(
				2,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			glLineWidth(3.3f);
			glDrawArrays(GL_LINE_STRIP, 0, curveVertex.size());
			glDisableVertexAttribArray(2);
			glUseProgram(0);
		}
		else if (!PointCurveMode)
		{
			if (show2)
			{
				curveVertex = curveVertexs[1];
				curveNormal = curveNormals[1];
				curveTan = curveTans[1];
				curveB = curveBs[1];
			}
			else if (show4)
			{
				curveVertex = curveVertexs[3];
				curveNormal = curveNormals[3];
				curveTan = curveTans[3];
				curveB = curveBs[3];
			}
			else if (show5)
			{
				curveVertex = curveVertexs[5];
				curveNormal = curveNormals[5];
				curveTan = curveTans[5];
				curveB = curveBs[5];
			}
			glUseProgram(program1);

			glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
			// black
			glUniform3f(ColorID, 0.0f, 0.0f, 0.0f);
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, curvevertexbuffer);
			glBufferData(GL_ARRAY_BUFFER,
				curveVertex.size() * sizeof(glm::vec3), &curveVertex[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(
				2,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			glLineWidth(3.3f);
			glDrawArrays(GL_LINE_STRIP, 0, curveVertex.size());

			// red
			glUniform3f(ColorID, 1.0f, 0.0f, 0.0f);
			glBufferData(GL_ARRAY_BUFFER,
				curveNormal.size() * sizeof(glm::vec3), &curveNormal[0], GL_STATIC_DRAW);
			glVertexAttribPointer(
				2,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			glLineWidth(3.3f);
			glDrawArrays(GL_LINES, 0, curveNormal.size());

			// green
			glUniform3f(ColorID, 0.0f, 1.0f, 0.0f);
			glBufferData(GL_ARRAY_BUFFER,
				curveTan.size() * sizeof(glm::vec3), &curveTan[0], GL_STATIC_DRAW);
			glVertexAttribPointer(
				2,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			glLineWidth(3.3f);
			glDrawArrays(GL_LINES, 0, curveTan.size());

			// blue
			glUniform3f(ColorID, 0.0f, 0.0f, 1.0f);
			glBufferData(GL_ARRAY_BUFFER,
				curveB.size() * sizeof(glm::vec3), &curveB[0], GL_STATIC_DRAW);
			glVertexAttribPointer(
				2,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			glLineWidth(3.3f);
			glDrawArrays(GL_LINES, 0, curveB.size());

			glDisableVertexAttribArray(2);
			glUseProgram(0);
		}


		glfwSwapBuffers(window);
	}

	glfwTerminate();
}

void calculateBezier(vector<float> points, vec3 up,int Interval, bool closed, bool sweep = false)
{
	viewModel();

	float mtx[16] = {
		1.0f / 3.0f,0,0,0,
		0,1.0f / 3.0f,0,0,
		0,0,1,0,
		0,0,0,1
	};
	mat4 invViewProjMatrix = glm::inverse(MVP) * glm::transpose(glm::make_mat4(mtx));
	vector<vec3> cp0, cp;
	for (int i = 0; i < points.size(); )
	{
		if (sweep)
		{
			cp.push_back(vec3(points[i], points[i + 1], points[i + 2]));
			cp0.push_back(vec3(points[i], points[i + 1], points[i + 2]));
			i += 3;
		}
		else
		{
			cp.push_back(invViewProjMatrix * vec4(points[i], points[i + 1], -1.0 + 0.01, 1));
			cp0.push_back(vec3(points[i], points[i + 1], 0));
			i += 2;
		}
	}
	if (closed && !isCounterClockWise(cp, up))
	{
		std::reverse(cp.begin(), cp.end());
		std::reverse(cp0.begin(), cp0.end());
	}

	if (closed)
	{
		cp.push_back(cp[0]);
	}

	curveVertexs.push_back(cp);

	// Calculate Bezier
	vector<vec3> o, o1, o2, o3;
	o.reserve(Interval);
	o1.reserve(Interval);
	o2.reserve(Interval);
	o3.reserve(Interval);
	vec3 lastB = up;
	if (sweep)
	{
		for (int i = 0; i < Interval; i++)
		{
			o.push_back(cubicBeziers(cp0, (float)i / Interval, closed));
			vec3 tmp = glm::normalize(getTan(cp0, (float)i / Interval, closed));
			o1.push_back(tmp);
			vec3 N = glm::normalize(glm::cross(tmp, lastB));
			o2.push_back(N);
			lastB = glm::normalize(glm::cross(N, tmp));
			o3.push_back(lastB);
		}
		o.push_back(o[0]);
		o1.push_back(o1[0]);
		o2.push_back(o2[0]);
		o3.push_back(o3[0]);
	}
	else
	{
		for (int i = 0; i <= Interval; i++)
		{
			o.push_back(cubicBeziers(cp0, (float)i / Interval, closed));
			vec3 tmp = glm::normalize(getTan(cp0, (float)i / Interval, closed));
			o1.push_back(tmp);
			vec3 N = glm::normalize(glm::cross(tmp, lastB));
			o2.push_back(N);
			lastB = glm::normalize(glm::cross(N, tmp));
			o3.push_back(lastB);
		}
	}

	curveVertexs.push_back(o);
	curveTans.push_back(o1);
	curveNormals.push_back(o2);
	curveBs.push_back(o3);
}

void generateCurveOut(int idx, int interval)
{
	const float ratio = 0.1;
	vector<vec3> o1, o2, o3;
	o1.reserve(interval * 2);
	o2.reserve(interval * 2);
	o3.reserve(interval * 2);
	for (int i = 0; i <= interval; i++)
	{
		o1.push_back(curveVertexs[idx][i]);
		o1.push_back(curveVertexs[idx][i] + ratio * curveTans[idx - 1][i]);
		o2.push_back(curveVertexs[idx][i]);
		o2.push_back(curveVertexs[idx][i] + ratio * curveNormals[idx - 1][i]);
		o3.push_back(curveVertexs[idx][i]);
		o3.push_back(curveVertexs[idx][i] + ratio * curveBs[idx - 1][i]);
	}
	curveTans.push_back(o1);
	curveNormals.push_back(o2);
	curveBs.push_back(o3);
}

void generateSOR(int idx)
{
	vector<vec3> o;
	vector<vec3> o1;
	vector<vec3> lastline = curveVertexs[idx];
	vector<vec3> lastNormalLine = curveNormals[idx - 1];
	float r = -glm::pi<float>() * 2.0f / angleInterval;
	mat4 roM = glm::toMat4(quat(
		cos(r / 2),
		0,
		sin(r / 2),
		0
	));
	for (int i = 1; i <= angleInterval; i++)
	{
		vector<vec3> thisline;
		vector<vec3> thisNormalLine;
		vec3 lastPoint = roM * vec4(lastline[0], 1.0);
		vec3 lastNormal = roM * vec4(lastNormalLine[0], 0);
		thisline.push_back(lastPoint);
		thisNormalLine.push_back(lastNormal);
		for (int j = 1; j <= ptInterval; j++)
		{
			vec3 thisPoint = roM * vec4(lastline[j], 1.0);
			vec3 thisNormal = roM * vec4(lastNormalLine[j], 0);
			thisline.push_back(thisPoint);
			thisNormalLine.push_back(thisNormal);
			o.push_back(lastPoint);
			o.push_back(thisPoint);
			o.push_back(lastline[j]);

			o.push_back(lastPoint);
			o.push_back(lastline[j]);
			o.push_back(lastline[j - 1]);

			o1.push_back(lastNormal);
			o1.push_back(thisNormal);
			o1.push_back(lastNormalLine[j]);

			o1.push_back(lastNormal);
			o1.push_back(lastNormalLine[j]);
			o1.push_back(lastNormalLine[j - 1]);
			lastPoint = thisPoint;
			lastNormal = thisNormal;
		}
		lastline = thisline;
		lastNormalLine = thisNormalLine;
	}
	faceVertexs.push_back(o);
	faceNormals.push_back(o1);
}

void generateSS(int idx1, int idx2)
{
	vector<vec3> o;
	vector<vec3> o1;
	vector<vec3> sweepCurve = curveVertexs[idx2];
	float resize[16] =
	{
		1.0f,0,0,0,
		0,1.0f ,0,0,
		0,0,1.0f,0,
		0,0,0,1
	};
	mat4 resizeM = glm::transpose( glm::make_mat4(resize));
	vector<vec3> profileCurve;
	for (int j = 0; j <= ptInterval; j++)
	{
		profileCurve.push_back(resizeM * vec4(curveVertexs[idx1][j], 1.0));
	}

	vector<vec3> lastline;
	vector<vec3> lastNormalLine;
	vec3 V = sweepCurve[0];
	vec3 coorX = curveNormals[idx2 - 1][0];
	vec3 coorY = curveBs[idx2 - 1][0];
	vec3 coorZ = curveTans[idx2 - 1][0];
	float coorMp[16] =
	{
		coorX.x,coorY.x,coorZ.x,V.x,
		coorX.y,coorY.y,coorZ.y,V.y,
		coorX.z,coorY.z,coorZ.z,V.z,
		0,0,0,1
	};
	float invcoorMp[9] =
	{
		coorX.x,coorY.x,coorZ.x,
		coorX.y,coorY.y,coorZ.y,
		coorX.z,coorY.z,coorZ.z
	};
	mat4 tranM = glm::transpose( glm::make_mat4(coorMp));
	mat3 inv = glm::inverse(glm::make_mat3(invcoorMp));
	for (int j = 0; j <= ptInterval; j++)
	{  
		vec4 tmp1 = tranM * vec4(profileCurve[j], 1.0);
		lastline.push_back(tmp1/tmp1.w);
		lastNormalLine.push_back(inv * profileCurve[j]);
	}

	for (int i = 1; i <= stInterval; i++)
	{
		vector<vec3> thisline;
		vector<vec3> thisNormalLine;

		vec3 V = sweepCurve[i];
		vec3 coorX = curveNormals[idx2 - 1][i];
		vec3 coorY = curveBs[idx2 - 1][i];
		vec3 coorZ = curveTans[idx2 - 1][i];
		float coorMp[16] =
		{
			coorX.x,coorY.x,coorZ.x,V.x,
			coorX.y,coorY.y,coorZ.y,V.y,
			coorX.z,coorY.z,coorZ.z,V.z,
			0,0,0,1
		};
		float invcoorMp[9] =
		{
			coorX.x,coorY.x,coorZ.x,
			coorX.y,coorY.y,coorZ.y,
			coorX.z,coorY.z,coorZ.z
		};
		mat4 tranM = glm::transpose(glm::make_mat4(coorMp));
		mat3 inv = glm::inverse(glm::make_mat3(invcoorMp));

		vec4 tmp1 = tranM * vec4(profileCurve[0], 1.0);
		vec3 lastPoint = tmp1 / tmp1.w;
		vec3 lastNormal = inv * profileCurve[0];
		thisline.push_back(lastPoint);
		thisNormalLine.push_back(lastNormal);
		for (int j = 1; j <= ptInterval; j++)
		{
			vec4 tmp1 = tranM * vec4(profileCurve[j], 1.0);
			vec3 thisPoint = tmp1 / tmp1.w;
			vec3 thisNormal = inv * profileCurve[j];
			thisline.push_back(thisPoint);
			thisNormalLine.push_back(thisNormal);
			o.push_back(lastPoint);
			o.push_back(thisPoint);
			o.push_back(lastline[j]);

			o.push_back(lastPoint);
			o.push_back(lastline[j]);
			o.push_back(lastline[j - 1]);

			o1.push_back(lastNormal);
			o1.push_back(thisNormal);
			o1.push_back(lastNormalLine[j]);

			o1.push_back(lastNormal);
			o1.push_back(lastNormalLine[j]);
			o1.push_back(lastNormalLine[j - 1]);
			lastPoint = thisPoint;
			lastNormal = thisNormal;
		}
		lastline = thisline;
		lastNormalLine = thisNormalLine;
	}
	faceVertexs.push_back(o);
	faceNormals.push_back(o1);
}

void data()
{
	calculateBezier(SORProfile, zview, ptInterval,false);
	generateCurveOut(1, ptInterval);
	calculateBezier(SSProfile, zview, ptInterval, true);
	generateCurveOut(3, ptInterval);
	calculateBezier(SSCurve, yview, stInterval, true, true);
	generateCurveOut(5, stInterval-1);

	generateSOR(1);
	generateSS(3, 5);
}

int main(int argc, char* argv[]) {
	data();
	init();
	loop();
	outputObjFile("SOR.obj", faceVertexs[0], faceNormals[0]);
	outputObjFile("SS.obj", faceVertexs[1], faceNormals[1]);
}