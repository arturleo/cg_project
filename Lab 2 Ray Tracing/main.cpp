#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <chrono>

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
using glm::mat4;
using glm::vec4;
using glm::vec3;
using std::cout;
using std::cin;
using std::endl;

int wwidth = 1024, wheight = 768;
GLFWwindow* window;

const float viewMoveSpeedRatio = 0.1f;
const float viewRotateSpeedRatio = 0.1f;

// controls
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
auto pressTime = std::chrono::high_resolution_clock::now();

// redraw
bool modified=false;
int frameNumber = 0;
//time
auto firstTime= std::chrono::high_resolution_clock::now();

glm::mat4 MVP;
glm::mat4 ModelMatrix = glm::mat4(1.0);
glm::mat4 ViewMatrix;
float vHorizontalAngle = -1.067;
float vVerticalAngle = -0.28;
glm::vec3 vPosition = glm::vec3(7.434, 4.477, -4.11);
glm::vec3 vDirection;
glm::vec3 vRight;
glm::vec3 vUp;
glm::mat4 ProjectionMatrix;
float zfar = 100;
float znear = 1;

// loading obj
bool isloadDDS = true;
string filename = "room_thickwalls" ;
Mesh mesh;
GLuint Texture;
GLuint vertexbuffer;
GLuint texturebuffer;
GLuint normalbuffer;
GLuint elementbuffer;

// quad
GLuint vao;
GLuint tex;
GLuint sampler;
GLuint quadProgram;
// compute shader
GLuint computeProgramID;
GLuint workGroupSizeX;
GLuint workGroupSizeY;
GLuint eyeUniform;
GLuint ray00Uniform;
GLuint ray10Uniform;
GLuint ray01Uniform;
GLuint ray11Uniform;
GLuint timeUniform;
GLuint blendFactorUniform;
GLuint bounceCountUniform;
GLuint framebufferImageBinding;

int bounceCount = 4;

void initShaders()
{
	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, wwidth, wheight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	quadProgram = LoadShaders("quad.vs.glsl", "quad.fs.glsl");
	glUseProgram(quadProgram);
	int texUniform = glGetUniformLocation(quadProgram, "tex");
	glUniform1i(texUniform, 0);
	glUseProgram(0);

	computeProgramID = glCreateProgram();
	GLuint cshader = glCreateShader(GL_COMPUTE_SHADER);
	LoadShader(cshader, "raytracer.cs.glsl");
	GLuint random = glCreateShader(GL_COMPUTE_SHADER);
	LoadShader(random, "random.cs.glsl");
	GLuint randomCommon = glCreateShader(GL_COMPUTE_SHADER);
	LoadShader(randomCommon, "randomCommon.cs.glsl");
	glAttachShader(computeProgramID, cshader);
	glAttachShader(computeProgramID, random);
	glAttachShader(computeProgramID, randomCommon);
	glLinkProgram(computeProgramID);
	GLint Result = GL_FALSE;
	int InfoLogLength;
	// Check the program
	glGetProgramiv(computeProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(computeProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(computeProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}
	glDetachShader(computeProgramID, cshader);
	glDetachShader(computeProgramID, random);
	glDetachShader(computeProgramID, randomCommon);
	glDeleteShader(cshader);
	glDeleteShader(random);
	glDeleteShader(randomCommon);

	glUseProgram(computeProgramID);
	GLint workGroupSize[3];
	glGetProgramiv(computeProgramID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	workGroupSizeX = workGroupSize[0];
	workGroupSizeY = workGroupSize[1];
	eyeUniform = glGetUniformLocation(computeProgramID, "eye");
	ray00Uniform = glGetUniformLocation(computeProgramID, "ray00");
	ray10Uniform = glGetUniformLocation(computeProgramID, "ray10");
	ray01Uniform = glGetUniformLocation(computeProgramID, "ray01");
	ray11Uniform = glGetUniformLocation(computeProgramID, "ray11");
	timeUniform = glGetUniformLocation(computeProgramID, "time");
	blendFactorUniform = glGetUniformLocation(computeProgramID, "blendFactor");
	bounceCountUniform = glGetUniformLocation(computeProgramID, "bounceCount");

	/* Query the "image binding point" of the image uniform */
	int params;
	int loc = glGetUniformLocation(computeProgramID, "framebufferImage");
	glGetUniformiv(computeProgramID, loc, &params);
	framebufferImageBinding = params;

	GLuint loadtexture = glGetUniformLocation(computeProgramID, "useTexture");
	glUniform1i(loadtexture, isloadDDS);
	glUseProgram(0);
}

void loadOBJtoPrg()
{
	string file = "res/" + filename + ".obj";
	if (!loadObj(isloadDDS,-1,-1, file.c_str() ,mesh))
	{
		fprintf(stderr, "failed to load obj file");
		getchar();
		exit(-1);
	}
	glUseProgram(computeProgramID);
	GLuint numberid = glGetUniformLocation(computeProgramID, "vn");
	glUniform1i(numberid, mesh.vn);
	numberid = glGetUniformLocation(computeProgramID, "fn");
	glUniform1i(numberid, mesh.fn);

	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 
		mesh.vs.size() * sizeof(glm::vec4), 
		&mesh.vs[0], 
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 
		mesh.vts.size() * sizeof(glm::vec2), 
		&mesh.vts[0], 
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		mesh.vns.size() * sizeof(glm::vec4),
		&mesh.vns[0],
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		mesh.fs.size() * sizeof(int),
		&mesh.fs[0],
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		mesh.maxvs.size() * sizeof(glm::vec4),
		&mesh.maxvs[0],
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		mesh.minvs.size() * sizeof(glm::vec4),
		&mesh.minvs[0],
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		mesh.ns.size() * sizeof(glm::vec4),
		&mesh.ns[0],
		GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind


	if (isloadDDS)
	{
		file = "res/"+filename+".DDS";
		Texture = loadDDS(file.c_str());
		GLuint TextureID = glGetUniformLocation(computeProgramID, "textureSampler");
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0+1);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(TextureID, 1);
	}
	glUseProgram(0);
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
	glfwWindowHint(GLFW_RESIZABLE, true);

	window = glfwCreateWindow(wwidth, wheight, "exp02", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}

	printf("press 'esc' to ternimate program\n"
		"press 'wasdqe' to move object\n"
		"press 'fghtry' to rotate object\n");
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mode) {
		if (action == GLFW_PRESS)
		{
			modified = true;
			pressTime = std::chrono::high_resolution_clock::now();
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
			modified = false;
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

	initShaders();
	loadOBJtoPrg();

	firstTime = std::chrono::high_resolution_clock::now();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glfwShowWindow(window);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, wwidth, wheight);
	glfwSwapBuffers(window);
}


void viewModel()
{
	ProjectionMatrix = glm::perspective(
		glm::radians(60.0f), (float)wwidth / wheight, znear, zfar);

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

	auto thisTime = std::chrono::high_resolution_clock::now();
	float elapsedSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(thisTime-pressTime).count() / 1E9f;
	float viewMoveSpeed = elapsedSeconds * viewMoveSpeedRatio;
	float viewRotateSpeed = elapsedSeconds * viewRotateSpeedRatio;
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

void loop()
{
	// reference: LWJGL demo/raytracing
	while (escPress != true && glfwWindowShouldClose(window) == 0)
	{
		glfwPollEvents();
		glViewport(0, 0, wwidth, wheight);
		// computeProgramID
		glUseProgram(computeProgramID);
		viewModel();

		if (modified) {
			frameNumber = 0;
		}
		mat4 invViewProjMatrix = glm::inverse(MVP);

		//time
		auto thisTime = std::chrono::high_resolution_clock::now();
		float elapsedSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(thisTime-firstTime).count() / 1E9f;
		glUniform1f(timeUniform, elapsedSeconds);

		/*
		 * We are going to average multiple successive frames, so here we
		 * compute the blend factor between old frame and new frame. 0.0 - use
		 * only the new frame > 0.0 - blend between old frame and new frame
		 */
		float blendFactor =  frameNumber / (frameNumber + 1.0f);
		glUniform1f(blendFactorUniform, blendFactor);
		glUniform1i(bounceCountUniform, bounceCount);

		/* Set viewing frustum corner rays in shader */
		float wsize = 1;
		vec3 tmpVector;
		glUniform3f(eyeUniform, vPosition.x, vPosition.y, vPosition.z);
		tmpVector=vec3(-1, -1, -1);
		tmpVector = glm::vec3(invViewProjMatrix * vec4(tmpVector, 1.0)) - vPosition;
		glUniform3f(ray00Uniform, tmpVector.x, tmpVector.y, tmpVector.z);
		tmpVector = vec3(-1, 1 ,-1);
		tmpVector = glm::vec3(invViewProjMatrix * vec4(tmpVector, 1.0)) - vPosition;
		glUniform3f(ray01Uniform, tmpVector.x, tmpVector.y, tmpVector.z);
		tmpVector = vec3(1, -1, -1);
		tmpVector = glm::vec3(invViewProjMatrix * vec4(tmpVector, 1.0)) - vPosition;
		glUniform3f(ray10Uniform, tmpVector.x, tmpVector.y, tmpVector.z);
		tmpVector = vec3(1, 1, -1);
		tmpVector = glm::vec3(invViewProjMatrix * vec4(tmpVector, 1.0)) - vPosition;
		glUniform3f(ray11Uniform, tmpVector.x, tmpVector.y, tmpVector.z);

		/* Bind level 0 of framebuffer texture as writable image in the shader. */
		glBindImageTexture(framebufferImageBinding, tex, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
		/*
		 * Compute appropriate global work size dimensions.
		 */
		int numGroupsX = (int)ceil((double)wwidth / workGroupSizeX);
		int numGroupsY = (int)ceil((double)wheight / workGroupSizeY);

		/* Invoke the compute shader. */
		glDispatchCompute(numGroupsX, numGroupsY, 1);
		/*
		 * Synchronize all writes to the framebuffer image before we let OpenGL
		 * source texels from it afterwards when rendering the final image with
		 * the full-screen quad.
		 */
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		/* Reset bindings. */
		glBindImageTexture(framebufferImageBinding, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
		glUseProgram(0);

		frameNumber++;

		//quadProgram
		glUseProgram(quadProgram);
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindSampler(0, sampler);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// unbind
		glBindSampler(0, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window);
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
	cout << "filename:" << filename << " isloadtexture:" << isloadDDS << endl;

	string bnum;
	cout << "please input bounce count:" << endl;
	getline(std::cin, bnum);
	if (!bnum.empty())
	{
		bounceCount = std::stoi(bnum);
		if (bounceCount <= 0)
			bounceCount = 1;
	}
	cout << "bounce count:" << bounceCount << endl;
	init();
	loop();
}