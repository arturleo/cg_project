#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

#include <GLFW/glfw3.h>

#include "utils/loadTexture.h"
#include "utils/meshio.h"

#pragma comment( lib, "OpenGL32.lib" )
using std::string;
using glm::quat;
using std::cout;
using std::cin;
using std::endl;

int wwidth = 1024, wheight = 768;

const int animateTime = 70;
const float viewMoveSpeed = 0.04f;
const float editMoveSpeed = 0.04f;
const float viewRotateSpeed = 0.01f;
const float editRotateSpeed = 0.025f;

glm::vec3 mTranslate = glm::vec3(0, 0, 0);
float vHorizontalAngle = -1.067;
float vVerticalAngle = -0.28;
glm::vec3 vPosition = glm::vec3(7.434, 4.477, -4.11);
glm::vec3 vDirection;
glm::vec3 vRight;
glm::vec3 vUp;

// loading obj
bool isloadDDS = true;
string filename = "room_thickwalls";
Mesh mesh;
GLuint Texture;
GLuint vertexbuffer;
GLuint texturebuffer;
GLuint normalbuffer;
GLuint elementbuffer;

// view matrix
glm::mat4 MVP;
glm::mat4 ViewMatrix;
glm::mat4 ModelMatrix = glm::mat4(1.0);
glm::mat4 ModelTransMatrix = glm::mat4(1.0);
quat ModelRoQuat = quat(1, 0, 0, 0);

// model rotation and moving
glm::mat4 oldModelTransMatrix = glm::mat4(1.0);
quat oldModelRotateQuat = quat(1, 0, 0, 0);
glm::mat4 atransMatrix;
quat interporateModelRotateQuat = quat(1, 0, 0, 0);


GLFWwindow* window;
// view
GLuint program0;
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;

//animation
int animateCounter = 0;
// user control temp var
bool modified = false;

bool escPress = false;
bool viewMode = true;
bool animateMode = false;

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

void initShaders()
{
	MatrixID = glGetUniformLocation(program0, "MVP");
	ViewMatrixID = glGetUniformLocation(program0, "V");
	ModelMatrixID = glGetUniformLocation(program0, "M");

	glUseProgram(program0);
	GLuint loadtexture = glGetUniformLocation(program0, "useTexture");
	glUniform1i(loadtexture, isloadDDS);
	glUseProgram(0);

}

void loadOBJtoPrg()
{
	string file = "res/" + filename + ".DDS";
	if (isloadDDS)
	{
		Texture = loadDDS(file.c_str());
		GLuint TextureID = glGetUniformLocation(program0, "textureSampler");
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(program0);
		glUniform1i(TextureID, 1);
		glUseProgram(0);
	}

	file = "res/" + filename + ".obj";
	if (!loadObj(isloadDDS, -1, -1, file.c_str(), mesh))
	{
		fprintf(stderr, "failed to load obj file");
		getchar();
		exit(-1);
	}

	// Load it into a VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		mesh.vs.size() * sizeof(glm::vec3), &mesh.vs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &texturebuffer);
	glBindBuffer(GL_ARRAY_BUFFER, texturebuffer);
	glBufferData(GL_ARRAY_BUFFER,
		mesh.vts.size() * sizeof(glm::vec2), &mesh.vts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		mesh.vns.size() * sizeof(glm::vec3), &mesh.vns[0], GL_STATIC_DRAW);

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		mesh.fs.size() * sizeof(unsigned short), &mesh.fs[0], GL_STATIC_DRAW);

	// VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, texturebuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
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

	window = glfwCreateWindow(wwidth, wheight, "exp00", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}

	printf("press '1' to toggle edit mode\n"
		"press again to finish editing and the play the animation\n"
		"press 'esc' to ternimate program\n"
		"press 'wasdqe' to move object or camera depending on corrent mode\n"
		"press 'fghtry' to rotate object or camera depending on corrent mode.\n");
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		if (action == GLFW_PRESS)
		{
			if (key == GLFW_KEY_ESCAPE)
			{
				escPress = true;
			}
			else if (key == GLFW_KEY_1)
			{
				if (viewMode)
				{
					interporateModelRotateQuat = ModelRoQuat;
					oldModelTransMatrix = ModelTransMatrix;
					oldModelRotateQuat = ModelRoQuat;
					printf("entering edit mode\n");
				}
				else if (modified)
				{
					printf("exiting edit mode\n");
					printf("starting animation\n");
					atransMatrix = glm::translate(
						glm::mat4(1.0f),
						mTranslate * (float)(1 / (float)animateTime));

					ModelTransMatrix = oldModelTransMatrix;
					ModelRoQuat = oldModelRotateQuat;
					animateMode = true;

					mTranslate = glm::vec3(0, 0, 0);
				}
				else
				{
					printf("exiting edit mode\n");
				}
				viewMode = !viewMode;
			}
			else if (!animateMode || animateMode && viewMode)
			{
				if (!viewMode)
				{
					modified = true;
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
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(-1);
	};

	program0 = LoadShaders("object.vs.glsl", "object.fs.glsl");
	initShaders();
	loadOBJtoPrg();

	glfwShowWindow(window);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
}


quat slerp(quat v0, quat v1, float t) {
	v0 = glm::normalize(v0);
	v1 = glm::normalize(v1);


	float dot = glm::dot(v0, v1);

	if (dot < 0.0f) {
		v1 = -v1;
		dot = -dot;
	}

	const float DOT_THRESHOLD = 0.9995;
	if (dot > DOT_THRESHOLD) {
		quat result = glm::mix(v0, v1, t);
		result = glm::normalize(result);
		return result;
	}

	float theta_0 = acos(dot);
	float theta = theta_0 * t;
	float sin_theta = sin(theta);
	float sin_theta_0 = sin(theta_0);

	float s0 = cos(theta) - dot * sin_theta / sin_theta_0;
	float s1 = sin_theta / sin_theta_0;

	quat tmp = glm::normalize(glm::mix(v0, v1, s1 / (s0 + s1)));

	return tmp;
}


void viewModel()
{
	glm::mat4 ProjectionMatrix = glm::perspective(
		glm::radians(60.0f), (float)wwidth / wheight, 1.0f, 100.0f);


	if (animateMode)
	{
		if (animateCounter < animateTime)
		{
			ModelTransMatrix = ModelTransMatrix * atransMatrix;
			ModelMatrix = ModelTransMatrix * glm::toMat4(slerp(ModelRoQuat, interporateModelRotateQuat, (float)animateCounter / animateTime));
			animateCounter++;
		}
		else
		{
			printf("finishing animation\n");
			animateMode = false;
			animateCounter = 0;
			ModelRoQuat = interporateModelRotateQuat;
		}
	}

	vDirection = glm::vec3(
		cos(vVerticalAngle) * sin(vHorizontalAngle),
		sin(vVerticalAngle),
		cos(vVerticalAngle) * cos(vHorizontalAngle)
	);

	vRight = glm::vec3(
		sin(vHorizontalAngle - glm::pi<float>() / 2.0f),
		0,
		cos(vHorizontalAngle - glm::pi<float>() / 2.0f)
	);

	vUp = glm::cross(vRight, vDirection);

	if (!animateMode && !viewMode)
	{
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

		ModelTransMatrix = glm::translate(glm::mat4(1.0f), mTranslate);

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

		interporateModelRotateQuat =
			ModelDirectionQuaternion *
			ModelUpQuaternion *
			ModelRightQuaternion *
			interporateModelRotateQuat;
		modelRightAngle = modelUpAngle = modelDirectionAngle = 0;
		ModelMatrix = oldModelTransMatrix * ModelTransMatrix * glm::toMat4(interporateModelRotateQuat);
	}
	else if (viewMode)
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

		vDirection = glm::vec3(
			cos(vVerticalAngle) * sin(vHorizontalAngle),
			sin(vVerticalAngle),
			cos(vVerticalAngle) * cos(vHorizontalAngle)
		);

		vRight = glm::vec3(
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
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// start edit objects in program
		glUseProgram(program0);

		viewModel();

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		glDrawElements(
			GL_TRIANGLES,      // mode
			mesh.fs.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glfwSwapBuffers(window);
		glUseProgram(0);
	}

	glfwTerminate();
}

int main(int argc, char* argv[]) {
	string yn, filen;
	cout << "please input filename:" << endl;
	getline(std::cin, filen);
	if (!filen.empty())
	{
		filename = filen;
		cout << "Does it has texture? please input y/n" << endl;
		getline(std::cin, yn);
		if (!yn.empty())
		{
			if (yn.c_str()[0] == 'y')
				isloadDDS = true;
			else if (yn.c_str()[0] == 'n')
				isloadDDS = false;
			else
			{
				cout << "please input y/n" << endl;
				getchar();
				exit(-1);
			}
		}
	}
	cout << "filename:" <<filename<<" isloadtexture:"<< isloadDDS << endl;
	init();
	loop();
}