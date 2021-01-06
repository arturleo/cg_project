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
using std::vector;
using glm::mat4;

int wwidth = 1024, wheight = 768;
GLFWwindow* window;


const int runTime = 70;
const float viewMoveSpeed = 0.04f;
const float viewRotateSpeed = 0.01f;

// running
int currentRun = 0;

bool escPress = false;
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


glm::mat4 MVP;
glm::mat4 ViewMatrix;
glm::mat4 ModelMatrix = glm::mat4(1.0);

float vHorizontalAngle = -glm::pi<float>();
float vVerticalAngle = 0;
glm::vec3 vPosition = glm::vec3(7, 5, 15);
glm::vec3 vDirection;
glm::vec3 vRight;
glm::vec3 vUp;

float modelDirectionAngle = 0.0f;
float modelRightAngle = 0.0f;
float modelUpAngle = 0.0f;

GLuint program0;
// MVP
GLuint MatrixID;
//V
GLuint ViewMatrixID;
//M
GLuint ModelMatrixID;

// loading obj
bool isloadDDS = false;
string filename = "01";
Mesh mesh;
vector<Mesh> meshes;
GLuint Texture;
GLuint vertexbuffer;
GLuint texturebuffer;
GLuint normalbuffer;
GLuint elementbuffer;
GLuint vao;

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
	string file = "res/" + filename + ".obj";
	if (!loadObj(isloadDDS, file.c_str(), meshes, mesh))
	{
		fprintf(stderr, "failed to load obj file");
		getchar();
		exit(-1);
	}

	// Load it into a VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		mesh.vs.size() * sizeof(glm::vec3), &mesh.vs[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &texturebuffer);
	glBindBuffer(GL_ARRAY_BUFFER, texturebuffer);
	glBufferData(GL_ARRAY_BUFFER,
		mesh.vts.size() * sizeof(glm::vec2), &mesh.vts[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		mesh.vns.size() * sizeof(glm::vec3), &mesh.vns[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		mesh.fs.size() * sizeof(unsigned short), &mesh.fs[0], GL_DYNAMIC_DRAW);

	// VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if (isloadDDS)
	{
		file = "res/" + filename + ".DDS";
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

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
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

	window = glfwCreateWindow(wwidth, wheight, "exp01", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}

	printf("press 'esc' to ternimate program\n"
		"press 'wasdqe' to move camera\n"
		"press 'fghtry' to rotate camera\n");
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		if (action == GLFW_PRESS)
		{
			if (key == GLFW_KEY_ESCAPE)
			{
				escPress = true;
			}
			else if (key == GLFW_KEY_A)
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
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(-1);
	};

	// enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, NULL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	program0 = LoadShaders("object.vs.glsl", "object.fs.glsl");
	initShaders();
	loadOBJtoPrg();

	glfwShowWindow(window);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
}



void viewModel()
{
	glm::mat4 ProjectionMatrix = glm::perspective(
		glm::radians(60.0f), (float)wwidth / wheight, 1.0f, 100.0f);

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


	ViewMatrix = glm::lookAt(
		vPosition,
		vPosition + vDirection,
		vUp
	);

	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
}

Mesh moveMeshMat(Mesh mesh, mat4 m)
{

	for (int i = 0; i < mesh.vn; i++)
	{
		mesh.vs[i] = m * vec4(mesh.vs[i], 1);
		mesh.vns[i] = m * vec4(mesh.vns[i], 0);
	}
	return mesh;
}


void moveModel()
{
	currentRun += 1;
	float t = (float)(currentRun % runTime) / runTime;
	// 0 lefthip leftknee leftankle
	// -0.4,5.7,-0.33	0,3.11,-0.2	0.2,0.63,-0.16
	// 3 neck head body
	// 6 righthip rightknee rightankle
	// -0.4,5.7,0.5	0,3.11,0.3	0.2,0.63,0.35
	// 9 leftshoulder leftelbow leftwrist
	// 0,8.37,-1.22	0,6.37,-1.2	0,4.36,-1.11
	// 12 rightelbow rightshoulder rightwrist
	//  0,6.36,1.40   0,8.37,1.44	0,4.36,1.33

	//	left arm 
	float armAngle0 = glm::pi<float>() * 1 / 4.0;
	float armAngle1 = glm::pi<float>() * 1 / 2.0;
	float armAngle2 = glm::pi<float>() * 2 / 3.0;
	mat4 r = glm::toMat4(glm::quat(
		cos(armAngle0 * (abs(0.5 - t)-0.25) / 2),
		0,
		0,
		1 * sin(armAngle0 * (abs(0.5 - t) - 0.25) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 4.36, -1.11)) 
		* r 
		* glm::translate(mat4(1.0), -vec3(0, 4.36, -1.11));
	Mesh n0 = moveMeshMat(meshes[11], r);

	n0 = merge(meshes[10], n0);
	r = glm::toMat4(glm::quat(
		cos(armAngle1 * abs(0.5 - t) / 2),
		0,
		0,
		1 * sin(armAngle1 * abs(0.5 - t) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 6.37, -1.2))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 6.37, -1.2));
	n0 = moveMeshMat(n0, r);
	n0 = merge(meshes[9], n0);
	r = glm::toMat4(glm::quat(
		cos(armAngle2 * (abs(0.5 - t) - 0.25) / 2),
		0,
		0,
		1 * sin(armAngle2 * (abs(0.5 - t) - 0.25) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 8.37, -1.22))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 8.37, -1.22));
	n0 = moveMeshMat(n0, r);

	//right leg
	float legAngle0 = glm::pi<float>() *1.0/2;
	float legAngle1 = glm::pi<float>() * 1 / 3.0;
	float legAngle2 = glm::pi<float>() * 4 / 4.0;
	r = glm::toMat4(glm::quat(
		cos(legAngle0 * (0.25 - abs(0.5 - t)) / 2),
		0,
		0,
		1 * sin(legAngle0 * (0.25 - abs(0.5 - t)) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0.2, 0.63, 0.35))
		* r
		* glm::translate(mat4(1.0), -vec3(0.2, 0.63, 0.35));
	Mesh n1 = moveMeshMat(meshes[8], r);

	n1 = merge(meshes[7], n1);
	r = glm::toMat4(glm::quat(
		cos(legAngle1 * (abs(0.5 - t) - 0.5) / 2),
		0,
		0,
		1 * sin(legAngle1 * (abs(0.5 - t) - 0.5) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 3.11, 0.3))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 3.11, 0.3));
	n1 = moveMeshMat(n1, r);

	n1 = merge(meshes[6], n1);
	r = glm::toMat4(glm::quat(
		cos(legAngle2 * (abs(0.5 - t) - 0.25) / 2),
		0,
		0,
		1 * sin(legAngle2 * (abs(0.5 - t) - 0.25) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(-0.4, 5.7, 0.5))
		* r
		* glm::translate(mat4(1.0), -vec3(-0.4, 5.7, 0.5));
	n1 = moveMeshMat(n1, r);
	n0 = merge(n0, n1);
	//right arm
	r = glm::toMat4(glm::quat(
		cos(armAngle0 * abs(0.25 - t)/ 2),
		0,
		0,
		1 * sin(armAngle0 * abs(0.25 - t) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 4.36, 1.33))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 4.36, 1.33));
	Mesh n2 = moveMeshMat(meshes[14], r);

	n2 = merge(meshes[12], n2);
	r = glm::toMat4(glm::quat(
		cos(armAngle1 * (0.5-abs(0.5 - t)) / 2),
		0,
		0,
		1 * sin(armAngle1 * (0.5 - abs(0.5 - t)) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 6.36, 1.40))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 6.36, 1.40));
	n2 = moveMeshMat(n2, r);

	n2 = merge(meshes[13], n2);
	r = glm::toMat4(glm::quat(
		cos(armAngle2 * (0.25 - abs(0.5 - t)) / 2),
		0,
		0,
		1 * sin(armAngle2 * (0.25 - abs(0.5 - t)) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 8.37, 1.44))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 8.37, 1.44));
	n2 = moveMeshMat(n2, r);
	n0 = merge(n0, n2);
	
	// left leg
	r = glm::toMat4(glm::quat(
		cos(legAngle0 * (-0.25 + abs(0.5 - t)) / 2),
		0,
		0,
		1 * sin(legAngle0 * (-0.25 + abs(0.5 - t)) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0.2, 0.63, -0.16))
		* r
		* glm::translate(mat4(1.0), -vec3(0.2, 0.63, -0.16));
	Mesh n3 = moveMeshMat(meshes[2], r);

	n3 = merge(meshes[1], n3);
	r = glm::toMat4(glm::quat(
		cos(legAngle1 * -abs(0.5 - t) / 2),
		0,
		0,
		1 * sin(legAngle1 * -abs(0.5 - t) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(0, 3.11, -0.2))
		* r
		* glm::translate(mat4(1.0), -vec3(0, 3.11, -0.2));
	n3 = moveMeshMat(n3, r);

	n3 = merge(meshes[0], n3);
	r = glm::toMat4(glm::quat(
		cos(legAngle2 * (-abs(0.5 - t) + 0.25) / 2),
		0,
		0,
		1 * sin(legAngle2 * (-abs(0.5 - t) + 0.25) / 2)
	));
	r = glm::translate(mat4(1.0), vec3(-0.4, 5.7, -0.33))
		* r
		* glm::translate(mat4(1.0), -vec3(-0.4, 5.7, -0.33));
	n3 = moveMeshMat(n3, r);
	n0 = merge(n0, n3);
	//body
	n0 = merge(n0, meshes[3]);
	n0 = merge(n0, meshes[4]);
	n0 = merge(n0, meshes[5]);
	r = glm::translate(mat4(1.0), vec3(4.0 * fmod(( (float)currentRun / runTime ),5), 0, 0));
	n0=moveMeshMat(n0, r);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		n0.vs.size() * sizeof(glm::vec3), &n0.vs[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,
		n0.vns.size() * sizeof(glm::vec3), &n0.vns[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


}

void loop()
{

	while (escPress != true && glfwWindowShouldClose(window) == 0)
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program0);

		viewModel();
		moveModel();

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
	init();
	loop();
}