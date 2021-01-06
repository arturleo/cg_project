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
	int vn=0;
	int fn=0;
	vector<vec3> vs;
	vector<vec2> vts;
	vector<vec3> vns;
	vector<short> fs;
};

Mesh merge(Mesh a, Mesh b);

bool loadObj(
	bool hasDDS,
	const char* path,
	vector<Mesh>& meshes,
	Mesh& mesh
);

void LoadShader(GLuint ShaderID, const char* file_path);

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path); 