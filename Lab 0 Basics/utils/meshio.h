#pragma once
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <stdlib.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// Include AssImp
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
using std::vector;
using glm::vec2;
using glm::vec3;
using glm::vec4;

struct Mesh
{
	int vn;
	int fn;
	vector<vec3> vs;
	vector<vec2> vts;
	vector<vec3> vns;
	vector<short> fs;
	vector<vec3> maxvs;
	vector<vec3>  minvs;
	vector<vec3>  ns;
};

bool loadObj(
	bool hasDDS,
	int start0,
	int end0,
	const char* path,
	Mesh& mesh
);

bool loadAssImp(
	const char* path,
	std::vector<unsigned short>& indices,
	std::vector<glm::vec3>& vertices,
	std::vector<glm::vec2>& uvs,
	std::vector<glm::vec3>& normals
);

void LoadShader(GLuint ShaderID, const char* file_path);

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path); 